#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_SurvivorHUD.generated.h"

UCLASS()
class TEAM02_API UUW_SurvivorHUD : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ShowItemBar();


    UFUNCTION(BlueprintCallable, Category = "HUD")
    void HideItemBar();


    UFUNCTION(BlueprintCallable, Category = "HUD")
    void SetItemPercent(float Percent);

protected:

    UPROPERTY(meta = (BindWidget))
    class UProgressBar* HP;

    UPROPERTY(meta = (BindWidget))
    class UProgressBar* Item;

private:

    UFUNCTION()
    void UpdateHP(float CurrentHP, float MaxHP);


    FDelegateHandle HPChangedHandle;
};