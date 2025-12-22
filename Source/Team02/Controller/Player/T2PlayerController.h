#pragma once

#include "CoreMinimal.h"
#include "Controller/T2BaseController.h"
#include "T2PlayerController.generated.h"

class UUW_SurvivorHUD;
class UUW_RoundProgressBar;

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

public:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUW_RoundProgressBar> InteractWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TObjectPtr<UUW_RoundProgressBar> InteractWidgetClassInstance;

    UPROPERTY()
    UMaterialInstanceDynamic* InteractMID;

private:
    void ShowSurvivorHUD();

public:

    void StartSpectate(AActor* Target);
    void NextSpectate();
    void ShowSpectatorUI();

    void StartInteractUI();
    void UpdateInteractUI(float Percent);
    void StopInteractUI();

    bool bInteractUIActive = false;
};