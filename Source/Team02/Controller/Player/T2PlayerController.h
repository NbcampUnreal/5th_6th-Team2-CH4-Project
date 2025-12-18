#pragma once

#include "CoreMinimal.h"
#include "Controller/T2BaseController.h"
#include "T2PlayerController.generated.h"

class UUW_SurvivorHUD;

UCLASS()
class TEAM02_API AT2PlayerController : public AT2BaseController
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;


    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUW_SurvivorHUD> SurvivorHUDWidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    TObjectPtr<UUW_SurvivorHUD> SurvivorHUDWidgetInstance;

private:
    void ShowSurvivorHUD();

public:

    void StartSpectate(AActor* Target);
    void NextSpectate();
    void ShowSpectatorUI();
};