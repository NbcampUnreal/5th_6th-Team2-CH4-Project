#include "T2PlayGameMod.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "PlayerState/T2PlayerState.h"
#include "Gimmick/ItemSpawner.h"
#include "EngineUtils.h"

AT2PlayGameMod::AT2PlayGameMod()
{
}

void AT2PlayGameMod::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority()) return;

    for (TActorIterator<AItemSpawner> It(GetWorld()); It; ++It)
    {
        AItemSpawner* Spawner = *It;

        if (IsValid(Spawner))
        {
            UE_LOG(LogTemp, Warning, TEXT("Found ItemSpawner, calling SpawnItems"));
            Spawner->SpawnItems();
        }
    }
}

void AT2PlayGameMod::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
    if (GetNumPlayers() >= RequiredPlayers)
    {
        ErrorMessage = TEXT("Server is full. Maximum 3 players allowed.");
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

    UE_LOG(LogTemp, Warning, TEXT("PostLogin - Player joined.  Total:  %d/%d"),
        ConnectedPlayers.Num(), RequiredPlayers);

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

    UE_LOG(LogTemp, Warning, TEXT("=== AssignRolesIfReady:  Assigning roles!  ==="));

    bRolesAssigned = true;

    TArray<APlayerController*> ShuffledPlayers = ConnectedPlayers;

    for (int32 i = ShuffledPlayers.Num() - 1; i > 0; --i)
    {
        int32 j = FMath::RandRange(0, i);
        ShuffledPlayers.Swap(i, j);
    }

    // 스폰 포인트 미리 가져오기
    TArray<AActor*> KillerSpawnPoints;
    TArray<AActor*> SurvivorSpawnPoints;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("KillerSpawn"), KillerSpawnPoints);
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("SurvivorSpawn"), SurvivorSpawnPoints);

    UE_LOG(LogTemp, Warning, TEXT("Found %d KillerSpawn, %d SurvivorSpawn points"),
        KillerSpawnPoints.Num(), SurvivorSpawnPoints.Num());

    int32 SurvivorCount = 0;
    int32 SurvivorSpawnIndex = 0;

    for (int32 i = 0; i < ShuffledPlayers.Num(); ++i)
    {
        APlayerController* PC = ShuffledPlayers[i];
        if (!PC) continue;

        AT2PlayerState* PS = PC->GetPlayerState<AT2PlayerState>();
        if (!PS) continue;

        EPlayerRole AssignedRole;
        AActor* SpawnPoint = nullptr;
        UClass* PawnClass = nullptr;

        if (i == 0)
        {
            AssignedRole = EPlayerRole::Killer;
            PawnClass = KillerClass;
            if (KillerSpawnPoints.Num() > 0)
            {
                SpawnPoint = KillerSpawnPoints[FMath::RandRange(0, KillerSpawnPoints.Num() - 1)];
            }
        }
        else
        {
            AssignedRole = EPlayerRole::Survivor;
            PawnClass = SurvivorClass;
            SurvivorCount++;
            if (SurvivorSpawnPoints.Num() > 0)
            {
                SpawnPoint = SurvivorSpawnPoints[SurvivorSpawnIndex % SurvivorSpawnPoints.Num()];
                SurvivorSpawnIndex++;
            }
        }

        PS->SetPlayerRole(AssignedRole);

        UE_LOG(LogTemp, Warning, TEXT("Player %d assigned:  %s (IsLocal: %s)"),
            i,
            AssignedRole == EPlayerRole::Killer ? TEXT("KILLER") : TEXT("SURVIVOR"),
            PC->IsLocalPlayerController() ? TEXT("YES") : TEXT("NO"));

        APawn* OldPawn = PC->GetPawn();
        if (OldPawn)
        {
            UE_LOG(LogTemp, Warning, TEXT("Player %d: Destroying old pawn:  %s"), i, *OldPawn->GetName());
            PC->UnPossess();
            OldPawn->Destroy();
        }

        // 직접 스폰
        if (SpawnPoint && PawnClass)
        {
            FVector SpawnLocation = SpawnPoint->GetActorLocation();
            FRotator SpawnRotation = SpawnPoint->GetActorRotation();

            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

            APawn* NewPawn = GetWorld()->SpawnActor<APawn>(
                PawnClass,
                SpawnLocation,
                SpawnRotation,
                SpawnParams
            );

            if (NewPawn)
            {
                PC->Possess(NewPawn);
                NewPawn->EnableInput(PC);
                UE_LOG(LogTemp, Warning, TEXT("Player %d:  Spawned at %s (%s), NewPawn = %s"),
                    i,
                    *SpawnPoint->GetName(),
                    AssignedRole == EPlayerRole::Killer ? TEXT("KillerSpawn") : TEXT("SurvivorSpawn"),
                    *NewPawn->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Player %d: Failed to spawn pawn! "), i);
            }
        }
        else
        {
            // 스폰 포인트나 클래스 없으면 기존 방식
            UE_LOG(LogTemp, Warning, TEXT("Player %d: No spawn point or class, using RestartPlayer"), i);
            RestartPlayer(PC);
            APawn* NewPawn = PC->GetPawn();
            if (NewPawn)
            {
                NewPawn->EnableInput(PC);
                UE_LOG(LogTemp, Warning, TEXT("Player %d: NewPawn = %s (fallback)"), i, *NewPawn->GetName());
            }
        }

        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;

        UE_LOG(LogTemp, Warning, TEXT("Player %d respawned and input configured"), i);
    }

    if (AT2PlayGameState* GS = GetGameState<AT2PlayGameState>())
    {
        GS->TotalSurvivors = SurvivorCount;
        GS->SurvivorsAlive = SurvivorCount;
        UE_LOG(LogTemp, Warning, TEXT("GameState: TotalSurvivors=%d, SurvivorsAlive=%d"), SurvivorCount, SurvivorCount);
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
        UE_LOG(LogTemp, Warning, TEXT("GetDefaultPawnClass:  Survivor"));
        return SurvivorClass;
    }

    UE_LOG(LogTemp, Warning, TEXT("GetDefaultPawnClass: Default (Role=None)"));
    return Super::GetDefaultPawnClassForController_Implementation(InController);
}

AActor* AT2PlayGameMod::ChoosePlayerStart_Implementation(AController* Player)
{
    EPlayerRole InRole = GetPlayerRole(Player);
    FName SpawnTag = (InRole == EPlayerRole::Killer) ? FName("KillerSpawn") : FName("SurvivorSpawn");

    UE_LOG(LogTemp, Warning, TEXT("ChoosePlayerStart:  Role = %d, Tag = %s"),
        (int32)InRole, *SpawnTag.ToString());

    TArray<AActor*> SpawnPoints;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), SpawnTag, SpawnPoints);

    UE_LOG(LogTemp, Warning, TEXT("ChoosePlayerStart: Found %d spawn points with tag %s"),
        SpawnPoints.Num(), *SpawnTag.ToString());

    if (SpawnPoints.Num() > 0)
    {
        int32 Index = FMath::RandRange(0, SpawnPoints.Num() - 1);
        UE_LOG(LogTemp, Warning, TEXT("ChoosePlayerStart: Using spawn point %s"),
            *SpawnPoints[Index]->GetName());
        return SpawnPoints[Index];
    }

    UE_LOG(LogTemp, Warning, TEXT("ChoosePlayerStart: No spawn points found, using default"));
    return Super::ChoosePlayerStart_Implementation(Player);
}


void AT2PlayGameMod::OnCharacterDead(APlayerController* DeadPlayerController)
{
    if (!HasAuthority()) return;

    UE_LOG(LogTemp, Warning, TEXT("=== OnCharacterDead Called ==="));

    // GameState에서 생존자 수 감소
    AT2PlayGameState* GS = GetGameState<AT2PlayGameState>();
    if (GS)
    {
        GS->OnSurvivorDied();
        UE_LOG(LogTemp, Warning, TEXT("SurvivorsAlive decreased to:  %d"), GS->SurvivorsAlive);
    }

    // 승패 조건 체크
    CheckWinConditions();
}

void AT2PlayGameMod::OnPlayerDied(APlayerController* Player)
{
    // OnCharacterDead로 통합
    OnCharacterDead(Player);
}

void AT2PlayGameMod::OnPlayerEscaped(APlayerController* Player)
{
    if (!HasAuthority()) return;

    UE_LOG(LogTemp, Warning, TEXT("Player Escaped! "));

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

    UE_LOG(LogTemp, Warning, TEXT("SurvivorsAlive: %d, SurvivorsEscaped: %d, TotalSurvivors: %d"),
        GS->SurvivorsAlive, GS->SurvivorsEscaped, GS->TotalSurvivors);

    // 모든 생존자가 죽거나 탈출했을 때만 게임 종료
    if (GS->SurvivorsAlive <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No survivors alive! "));

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

    UE_LOG(LogTemp, Warning, TEXT("Game still in progress - Survivors remaining: %d"), GS->SurvivorsAlive);
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

    UE_LOG(LogTemp, Warning, TEXT("=== MATCH ENDED: %s ==="), *ResultStr);

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
