#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "T2GameInstance.h"
#include "T2PlayGameMod.generated.h"

UCLASS()
class TEAM02_API AT2PlayGameMod : public AGameModeBase
{
    GENERATED_BODY()

public:
    AT2PlayGameMod();

protected:
    virtual void BeginPlay() override;
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;


    virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

    UPROPERTY(EditDefaultsOnly, Category = "Characters")
    TSubclassOf<APawn> KillerClass;

    UPROPERTY(EditDefaultsOnly, Category = "Characters")
    TSubclassOf<APawn> SurvivorClass;

private:
    EPlayerRole CurrentRole;
};