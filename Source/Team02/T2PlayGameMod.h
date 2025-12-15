#pragma once

#include "CoreMinimal.h"
#include "./GameMode/T2GameModeBase.h"
#include "T2GameInstance.h"
#include "T2PlayGameMod.generated.h"

UCLASS()
class TEAM02_API AT2PlayGameMod : public AGameModeBase
{
    GENERATED_BODY()

public:
    AT2PlayGameMod();

    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

protected:
    virtual void BeginPlay() override;
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
    virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

    UPROPERTY(EditDefaultsOnly, Category = "Characters")
    TSubclassOf<APawn> KillerClass;

    UPROPERTY(EditDefaultsOnly, Category = "Characters")
    TSubclassOf<APawn> SurvivorClass;

private:
    bool bKillerTaken = false;
    bool bSurvivorTaken = false;

    TMap<APlayerController*, EPlayerRole> PlayerRoles;

    EPlayerRole AssignRoleToPlayer(APlayerController* Player);
    EPlayerRole GetPlayerRoleFromMap(AController* Player);
};