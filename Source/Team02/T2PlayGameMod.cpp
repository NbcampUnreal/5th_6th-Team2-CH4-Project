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

void AT2PlayGameMod::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
    if (GetNumPlayers() >= RequiredPlayers)
    {
        ErrorMessage = TEXT("Server is full.  Maximum 3 players allowed.");
        UE_LOG(LogTemp, Warning, TEXT("PreLogin:  Rejected player - server full (%d/%d)"), GetNumPlayers(), RequiredPlayers);
        return;
    }

    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
}

void AT2PlayGameMod::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (!NewPlayer) return;

    ConnectedPlayers.AddUnique(NewPlayer);

    UE_LOG(LogTemp, Warning, TEXT("PostLogin - Player joined.  Total: %d/%d"),
        ConnectedPlayers.Num(), RequiredPlayers);

    // 3명 모이면 역할 배정
    AssignRolesIfReady();
}

void AT2PlayGameMod::AssignRolesIfReady()
{
    if (bRolesAssigned || ConnectedPlayers.Num() < RequiredPlayers)
    {
        return;
    }

    if (!HasAuthority())
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("=== AssignRolesIfReady: Assigning roles! ==="));

    bRolesAssigned = true;

    // 플레이어 목록 셔플 (랜덤 배정)
    TArray<APlayerController*> ShuffledPlayers = ConnectedPlayers;

    for (int32 i = ShuffledPlayers.Num() - 1; i > 0; --i)
    {
        int32 j = FMath::RandRange(0, i);
        ShuffledPlayers.Swap(i, j);
    }

    int32 SurvivorCount = 0;

    for (int32 i = 0; i < ShuffledPlayers.Num(); ++i)
    {
        APlayerController* PC = ShuffledPlayers[i];
        if (!PC) continue;

        AT2PlayerState* PS = PC->GetPlayerState<AT2PlayerState>();
        if (!PS) continue;

        EPlayerRole AssignedRole;

        if (i == 0)
        {
            AssignedRole = EPlayerRole::Killer;
        }
        else
        {
            AssignedRole = EPlayerRole::Survivor;
            SurvivorCount++;
        }

        PS->SetPlayerRole(AssignedRole);

        UE_LOG(LogTemp, Warning, TEXT("Player %d assigned: %s (IsLocal: %s)"),
            i,
            AssignedRole == EPlayerRole::Killer ? TEXT("KILLER") : TEXT("SURVIVOR"),
            PC->IsLocalPlayerController() ? TEXT("YES") : TEXT("NO"));

        // 기존 Pawn 제거
        APawn* OldPawn = PC->GetPawn();
        if (OldPawn)
        {
            UE_LOG(LogTemp, Warning, TEXT("Player %d:  Destroying old pawn:  %s"), i, *OldPawn->GetName());
            PC->UnPossess();
            OldPawn->Destroy();
        }

        // 새 Pawn으로 리스폰
        RestartPlayer(PC);

        // Possess 확인
        APawn* NewPawn = PC->GetPawn();
        if (NewPawn)
        {
            // 입력 활성화
            NewPawn->EnableInput(PC);

            UE_LOG(LogTemp, Warning, TEXT("Player %d:  NewPawn = %s"), i, *NewPawn->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Player %d: NO PAWN AFTER RESTART! "), i);
        }

        // 입력 모드 설정
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;

        UE_LOG(LogTemp, Warning, TEXT("Player %d respawned and input configured"), i);
    }

    // GameState에 생존자 수 설정
    if (AT2PlayGameState* GS = GetGameState<AT2PlayGameState>())
    {
        GS->TotalSurvivors = SurvivorCount;
        GS->SurvivorsAlive = SurvivorCount;
    }

    UE_LOG(LogTemp, Warning, TEXT("Roles assigned - Killer: 1, Survivors: %d"), SurvivorCount);
}

void AT2PlayGameMod::Logout(AController* Exiting)
{
    APlayerController* PC = Cast<APlayerController>(Exiting);
    if (PC)
    {
        ConnectedPlayers.Remove(PC);
    }

    Super::Logout(Exiting);
}

EPlayerRole AT2PlayGameMod::GetPlayerRole(AController* Player) const
{
    if (!Player) return EPlayerRole::None;

    if (APlayerController* PC = Cast<APlayerController>(Player))
    {
        if (AT2PlayerState* PS = PC->GetPlayerState<AT2PlayerState>())
        {
            return PS->PlayerRole;
        }
    }

    return EPlayerRole::None;
}

UClass* AT2PlayGameMod::GetDefaultPawnClassForController_Implementation(AController* InController)
{
    EPlayerRole InRole = GetPlayerRole(InController);

    if (InRole == EPlayerRole::Killer && KillerClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetDefaultPawnClass: Killer"));
        return KillerClass;
    }
    else if (InRole == EPlayerRole::Survivor && SurvivorClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetDefaultPawnClass: Survivor"));
        return SurvivorClass;
    }

    UE_LOG(LogTemp, Warning, TEXT("GetDefaultPawnClass: Default (Role=None)"));
    return Super::GetDefaultPawnClassForController_Implementation(InController);
}

AActor* AT2PlayGameMod::ChoosePlayerStart_Implementation(AController* Player)
{
    EPlayerRole InRole = GetPlayerRole(Player);
    FName SpawnTag = (InRole == EPlayerRole::Killer) ? FName("KillerSpawn") : FName("SurvivorSpawn");

    TArray<AActor*> SpawnPoints;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), SpawnTag, SpawnPoints);

    if (SpawnPoints.Num() > 0)
    {
        int32 Index = FMath::RandRange(0, SpawnPoints.Num() - 1);
        return SpawnPoints[Index];
    }

    return Super::ChoosePlayerStart_Implementation(Player);
}

void AT2PlayGameMod::OnPlayerDied(APlayerController* Player)
{
    if (!HasAuthority()) return;

    UE_LOG(LogTemp, Warning, TEXT("Player Died! "));

    if (AT2PlayGameState* GS = GetGameState<AT2PlayGameState>())
    {
        GS->OnSurvivorDied();
    }

    CheckWinConditions();
}

void AT2PlayGameMod::OnPlayerEscaped(APlayerController* Player)
{
    if (!HasAuthority()) return;

    UE_LOG(LogTemp, Warning, TEXT("Player Escaped!"));

    if (AT2PlayGameState* GS = GetGameState<AT2PlayGameState>())
    {
        GS->OnSurvivorEscaped();
    }

    CheckWinConditions();
}

void AT2PlayGameMod::CheckWinConditions()
{
    UE_LOG(LogTemp, Warning, TEXT("=== CheckWinConditions START ==="));

    if (bMatchEnded)
    {
        UE_LOG(LogTemp, Warning, TEXT("Match already ended, skipping"));
        return;
    }

    AT2PlayGameState* GS = GetGameState<AT2PlayGameState>();
    if (!GS)
    {
        UE_LOG(LogTemp, Error, TEXT("GameState is NULL!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("SurvivorsAlive: %d, SurvivorsEscaped: %d, TotalSurvivors:  %d"),
        GS->SurvivorsAlive, GS->SurvivorsEscaped, GS->TotalSurvivors);

    if (GS->SurvivorsAlive <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No survivors alive!"));

        if (GS->SurvivorsEscaped == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("No one escaped - Killer Wins!"));
            EndMatch(EMatchResult::KillerWin);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Some escaped - Survivor Wins!"));
            EndMatch(EMatchResult::SurvivorWin);
        }
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Game still in progress"));
}

void AT2PlayGameMod::EndMatch(EMatchResult Result)
{
    if (bMatchEnded) return;
    bMatchEnded = true;

    AT2PlayGameState* GS = GetGameState<AT2PlayGameState>();
    if (GS)
    {
        GS->SetMatchResult(Result);
    }

    FString ResultStr;
    switch (Result)
    {
    case EMatchResult::KillerWin:   ResultStr = TEXT("Killer Wins!"); break;
    case EMatchResult::SurvivorWin: ResultStr = TEXT("Survivors Win!"); break;
    default: ResultStr = TEXT("Draw"); break;
    }

    UE_LOG(LogTemp, Warning, TEXT("=== MATCH ENDED:  %s ==="), *ResultStr);

    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, [this]()
        {
            UWorld* World = GetWorld();
            if (World)
            {
                World->ServerTravel(TEXT("/Game/Library_Pack/Maps/Example? listen"));
            }
        }, 3.0f, false);
}