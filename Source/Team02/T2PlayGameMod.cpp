#include "T2PlayGameMod.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "PlayerState/T2PlayerState.h"

AT2PlayGameMod::AT2PlayGameMod()
{
}

void AT2PlayGameMod::BeginPlay()
{
    Super::BeginPlay();
}

APlayerController* AT2PlayGameMod::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
    EPlayerRole AssignedRole = AssignRoleForNewPlayer(Options);
    PendingRole = AssignedRole;

    TSubclassOf<APlayerController> ControllerClassToUse = nullptr;

    if (AssignedRole == EPlayerRole::Killer && KillerControllerClass)
    {
        ControllerClassToUse = KillerControllerClass;
    }
    else if (AssignedRole == EPlayerRole::Survivor && SurvivorControllerClass)
    {
        ControllerClassToUse = SurvivorControllerClass;
    }
    else
    {
        ControllerClassToUse = PlayerControllerClass;
    }

    TSubclassOf<APlayerController> OriginalClass = PlayerControllerClass;
    PlayerControllerClass = ControllerClassToUse;

    APlayerController* NewController = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);

    PlayerControllerClass = OriginalClass;
    PendingRole = EPlayerRole::None;

    if (NewController)
    {
        PlayerRoles.Add(NewController, AssignedRole);
    }

    return NewController;
}


EPlayerRole AT2PlayGameMod::AssignRoleForNewPlayer(const FString& Options)
{
    UT2GameInstance* GI = Cast<UT2GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

    UE_LOG(LogTemp, Warning, TEXT("=== AssignRole ==="));
    UE_LOG(LogTemp, Warning, TEXT("Before - Killers: %d/%d, Survivors: %d/%d"),
        CurrentKillers, MaxKillers, CurrentSurvivors, MaxSurvivors);

    EPlayerRole WantedRole = EPlayerRole::None;

    // 1. URL Options에서 역할 파싱
    FString RoleOption = UGameplayStatics::ParseOption(Options, TEXT("Role"));
    if (!RoleOption.IsEmpty())
    {
        if (RoleOption == TEXT("Killer"))
            WantedRole = EPlayerRole::Killer;
        else if (RoleOption == TEXT("Survivor"))
            WantedRole = EPlayerRole::Survivor;
    }

    // 2. GameInstance에서 확인 (Options가 없을 때만)
    if (WantedRole == EPlayerRole::None && GI && GI->SelectedRole != EPlayerRole::None)
    {
        WantedRole = GI->SelectedRole;
    }

    // 3. 희망 역할 배정 시도, 불가능하면 반대 역할
    EPlayerRole AssignedRole = EPlayerRole::None;

    if (WantedRole == EPlayerRole::Killer)
    {
        if (CurrentKillers < MaxKillers)
        {
            AssignedRole = EPlayerRole::Killer;
        }
        else if (CurrentSurvivors < MaxSurvivors)
        {
            // Killer 꽉 참 → Survivor로
            AssignedRole = EPlayerRole::Survivor;
            UE_LOG(LogTemp, Warning, TEXT("Killer full, assigned SURVIVOR instead"));
        }
    }
    else if (WantedRole == EPlayerRole::Survivor)
    {
        if (CurrentSurvivors < MaxSurvivors)
        {
            //Survivor 원하지만, Killer가 없으면 Killer로
            if (CurrentKillers < MaxKillers && CurrentKillers == 0)
            {
                // 아직 Killer가 없고, 내가 첫 번째가 아니면 Killer로
                // (첫 번째 = 호스트 = 희망대로)
                if (CurrentSurvivors > 0)
                {
                    AssignedRole = EPlayerRole::Killer;
                    UE_LOG(LogTemp, Warning, TEXT("No Killer yet, assigned KILLER instead"));
                }
                else
                {
                    // 첫 번째 접속자는 희망대로
                    AssignedRole = EPlayerRole::Survivor;
                }
            }
            else
            {
                AssignedRole = EPlayerRole::Survivor;
            }
        }
        else if (CurrentKillers < MaxKillers)
        {
            // Survivor 꽉 참 → Killer로
            AssignedRole = EPlayerRole::Killer;
            UE_LOG(LogTemp, Warning, TEXT("Survivor full, assigned KILLER instead"));
        }
    }
    else
    {
        // 희망 없음 → 자동 배정 (Killer 먼저)
        if (CurrentKillers < MaxKillers)
        {
            AssignedRole = EPlayerRole::Killer;
        }
        else if (CurrentSurvivors < MaxSurvivors)
        {
            AssignedRole = EPlayerRole::Survivor;
        }
    }

    // 카운트 증가
    if (AssignedRole == EPlayerRole::Killer)
    {
        CurrentKillers++;
    }
    else if (AssignedRole == EPlayerRole::Survivor)
    {
        CurrentSurvivors++;
    }

    UE_LOG(LogTemp, Warning, TEXT("After - Assigned: %s, Killers:  %d, Survivors: %d"),
        AssignedRole == EPlayerRole::Killer ? TEXT("KILLER") :
        AssignedRole == EPlayerRole::Survivor ? TEXT("SURVIVOR") : TEXT("NONE"),
        CurrentKillers, CurrentSurvivors);

    return AssignedRole;
}

EPlayerRole AT2PlayGameMod::GetPlayerRoleFromMap(AController* Player)
{
    if (PendingRole != EPlayerRole::None)
    {
        return PendingRole;
    }

    APlayerController* PC = Cast<APlayerController>(Player);
    if (PC && PlayerRoles.Contains(PC))
    {
        return PlayerRoles[PC];
    }
    return EPlayerRole::None;
}

void AT2PlayGameMod::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (!NewPlayer) return;


    if (AT2PlayerState* PS = NewPlayer->GetPlayerState<AT2PlayerState>())
    {
        EPlayerRole* FoundRole = PlayerRoles.Find(NewPlayer);
        if (FoundRole)
        {
            PS->SetPlayerRole(*FoundRole);
        }
    }

    //  GameState에 현재 생존자 수 설정 
    if (AT2PlayGameState* GS = GetGameState<AT2PlayGameState>())
    {
        GS->TotalSurvivors = CurrentSurvivors;
        GS->SurvivorsAlive = CurrentSurvivors;
    }

  
    FInputModeGameOnly InputMode;
    NewPlayer->SetInputMode(InputMode);
    NewPlayer->bShowMouseCursor = false;

    UE_LOG(LogTemp, Warning, TEXT("PostLogin - Total:  %d, Killers: %d, Survivors: %d"),
        TotalPlayers, CurrentKillers, CurrentSurvivors);
}

void AT2PlayGameMod::Logout(AController* Exiting)
{
    APlayerController* PC = Cast<APlayerController>(Exiting);
    if (PC && PlayerRoles.Contains(PC))
    {
        EPlayerRole ExitingRole = PlayerRoles[PC];


        if (ExitingRole == EPlayerRole::Killer)
        {
            CurrentKillers--;
        }
        else if (ExitingRole == EPlayerRole::Survivor)
        {
            CurrentSurvivors--;
        }

        PlayerRoles.Remove(PC);
    }

    Super::Logout(Exiting);
}

UClass* AT2PlayGameMod::GetDefaultPawnClassForController_Implementation(AController* InController)
{
    EPlayerRole FoundRole = GetPlayerRoleFromMap(InController);

    if (FoundRole == EPlayerRole::Killer && KillerClass)
    {
        return KillerClass;
    }
    else if (FoundRole == EPlayerRole::Survivor && SurvivorClass)
    {
        return SurvivorClass;
    }

    return Super::GetDefaultPawnClassForController_Implementation(InController);
}

AActor* AT2PlayGameMod::ChoosePlayerStart_Implementation(AController* Player)
{
    EPlayerRole FoundRole = GetPlayerRoleFromMap(Player);
    FName SpawnTag = (FoundRole == EPlayerRole::Killer) ? FName("KillerSpawn") : FName("SurvivorSpawn");

    TArray<AActor*> SpawnPoints;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), SpawnTag, SpawnPoints);

    if (SpawnPoints.Num() > 0)
    {
        return SpawnPoints[0];
    }

    return Super::ChoosePlayerStart_Implementation(Player);
}

void AT2PlayGameMod::OnPlayerDied(APlayerController* Player)
{
    if (!HasAuthority()) return;
    UE_LOG(LogTemp, Warning, TEXT("Player Died! "));
    CheckWinConditions();
}

void AT2PlayGameMod::OnPlayerEscaped(APlayerController* Player)
{
    if (!HasAuthority()) return;
    UE_LOG(LogTemp, Warning, TEXT("Player Escaped!"));
    CheckWinConditions();
}

void AT2PlayGameMod::CheckWinConditions()
{
    if (bMatchEnded) return;

    AT2PlayGameState* GS = GetGameState<AT2PlayGameState>();
    if (!GS) return;

    //  Killer 승리:  생존자 전원 사망 (0명)
    if (GS->SurvivorsAlive <= 0)
    {
        // 탈출한 사람이 없으면 Killer 완승
        if (GS->SurvivorsEscaped == 0)
        {
            EndMatch(EMatchResult::KillerWin);
        }
        else
        {
            // 일부 탈출했으면 Survivor 승리
            EndMatch(EMatchResult::SurvivorWin);
        }
        return;
    }

    // Survivor 승리: 전원 탈출
    if (GS->SurvivorsEscaped >= GS->TotalSurvivors)
    {
        EndMatch(EMatchResult::SurvivorWin);
        return;
    }
}

void AT2PlayGameMod::EndMatch(EMatchResult Result)
{
    if (bMatchEnded) return;
    bMatchEnded = true;

    FString ResultStr;
    switch (Result)
    {
    case EMatchResult::KillerWin:  ResultStr = TEXT("Killer Wins! "); break;
    case EMatchResult::SurvivorWin: ResultStr = TEXT("Survivors Win!"); break;
    default: ResultStr = TEXT("Draw"); break;
    }

    UE_LOG(LogTemp, Warning, TEXT("=== MATCH ENDED:  %s ==="), *ResultStr);
}