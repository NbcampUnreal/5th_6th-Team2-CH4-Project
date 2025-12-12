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


    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }
}


UClass* AT2PlayGameMod::GetDefaultPawnClassForController_Implementation(AController* InController)
{
    UT2GameInstance* GI = Cast<UT2GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    if (GI)
    {
        if (GI->SelectedRole == EPlayerRole::Killer && KillerClass)
        {
            return KillerClass;
        }
        else if (GI->SelectedRole == EPlayerRole::Survivor && SurvivorClass)
        {
            return SurvivorClass;
        }
    }

    return Super::GetDefaultPawnClassForController_Implementation(InController);
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

    TArray<AActor*> SpawnPoints;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), SpawnTag, SpawnPoints);

    if (SpawnPoints.Num() > 0)
    {
        return SpawnPoints[0];
    }

    return Super::ChoosePlayerStart_Implementation(Player);
}