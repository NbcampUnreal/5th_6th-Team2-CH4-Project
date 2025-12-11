#include "T2PlayGameMod.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

AT2PlayGameMod::AT2PlayGameMod()
{
    DefaultPawnClass = nullptr;
    CurrentRole = EPlayerRole::Survivor;
}

void AT2PlayGameMod::BeginPlay()
{
    Super::BeginPlay();

    UT2GameInstance* GI = Cast<UT2GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    if (GI)
    {
        CurrentRole = GI->SelectedRole;

        UE_LOG(LogTemp, Warning, TEXT("=== T2PlayGameMod BeginPlay ==="));
        UE_LOG(LogTemp, Warning, TEXT("Selected Role: %d (0=None, 1=Killer, 2=Survivor)"), (int32)CurrentRole);

        if (CurrentRole == EPlayerRole::Killer && KillerClass)
        {
            DefaultPawnClass = KillerClass;
            UE_LOG(LogTemp, Warning, TEXT("Spawning as KILLER"));
        }
        else if (CurrentRole == EPlayerRole::Survivor && SurvivorClass)
        {
            DefaultPawnClass = SurvivorClass;
            UE_LOG(LogTemp, Warning, TEXT("Spawning as SURVIVOR"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No valid class set!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameInstance Cast FAILED!"));
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        PC->bShowMouseCursor = false;
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);

        RestartPlayer(PC);
    }
}

AActor* AT2PlayGameMod::ChoosePlayerStart_Implementation(AController* Player)
{

    EPlayerRole RoleToUse = EPlayerRole::Survivor;

    UT2GameInstance* GI = Cast<UT2GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    if (GI)
    {
        RoleToUse = GI->SelectedRole;
    }

    FName SpawnTag = (RoleToUse == EPlayerRole::Killer) ? FName("KillerSpawn") : FName("SurvivorSpawn");

    UE_LOG(LogTemp, Warning, TEXT("ChoosePlayerStart - Role: %d, Tag: %s"), (int32)RoleToUse, *SpawnTag.ToString());

    TArray<AActor*> SpawnPoints;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), SpawnTag, SpawnPoints);

    UE_LOG(LogTemp, Warning, TEXT("Found %d spawn points"), SpawnPoints.Num());

    if (SpawnPoints.Num() > 0)
    {
        return SpawnPoints[0];
    }

    return Super::ChoosePlayerStart_Implementation(Player);
}