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

// ★ 함수 하나만!  (5명 기준 새 버전) ★
EPlayerRole AT2PlayGameMod::AssignRoleForNewPlayer(const FString& Options)
{
    UT2GameInstance* GI = Cast<UT2GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

    FString RoleOption = UGameplayStatics::ParseOption(Options, TEXT("Role"));

    if (!RoleOption.IsEmpty())
    {
        if (RoleOption == TEXT("Killer") && CurrentKillers < MaxKillers)
        {
            CurrentKillers++;
            return EPlayerRole::Killer;
        }
        else if (RoleOption == TEXT("Survivor") && CurrentSurvivors < MaxSurvivors)
        {
            CurrentSurvivors++;
            return EPlayerRole::Survivor;
        }
    }

    if (GI && GI->SelectedRole != EPlayerRole::None)
    {
        EPlayerRole WantedRole = GI->SelectedRole;

        if (WantedRole == EPlayerRole::Killer && CurrentKillers < MaxKillers)
        {
            CurrentKillers++;
            return EPlayerRole::Killer;
        }
        else if (WantedRole == EPlayerRole::Survivor && CurrentSurvivors < MaxSurvivors)
        {
            CurrentSurvivors++;
            return EPlayerRole::Survivor;
        }
    }

    // 자동 배정
    if (CurrentKillers < MaxKillers)
    {
        CurrentKillers++;
        return EPlayerRole::Killer;
    }
    else if (CurrentSurvivors < MaxSurvivors)
    {
        CurrentSurvivors++;
        return EPlayerRole::Survivor;
    }

    return EPlayerRole::None;
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

    FInputModeGameOnly InputMode;
    NewPlayer->SetInputMode(InputMode);
    NewPlayer->bShowMouseCursor = false;
}

void AT2PlayGameMod::Logout(AController* Exiting)
{
    APlayerController* PC = Cast<APlayerController>(Exiting);
    if (PC && PlayerRoles.Contains(PC))
    {
        EPlayerRole ExitingRole = PlayerRoles[PC];

        // ★ 새 변수 사용 ★
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

    // TODO: GameState 만들면 여기서 체크
    // AT2PlayGameState* GS = GetGameState<AT2PlayGameState>();
    // if (!GS) return;

    UE_LOG(LogTemp, Warning, TEXT("CheckWinConditions called"));
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