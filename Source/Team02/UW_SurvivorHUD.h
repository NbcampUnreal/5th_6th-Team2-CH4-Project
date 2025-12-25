#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_SurvivorHUD.generated.h"

class UImage;
class ASurvivorPlayerState;

UCLASS()
class TEAM02_API UUW_SurvivorHUD : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ShowBatteryBar();

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void HideBatteryBar();

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void SetBatteryPercent(float Percent);

protected:
    UPROPERTY(meta = (BindWidget))
    class UProgressBar* HP;

    UPROPERTY(meta = (BindWidget))
    class UProgressBar* Battery;

private:
    UFUNCTION()
    void UpdateHP(float CurrentHP, float MaxHP);

    UFUNCTION()
    void UpdateBattery(float CurrentBattery, float MaxBattery);

    void TryBindToPlayerState();

    FDelegateHandle HPChangedHandle;
    FDelegateHandle BatteryChangedHandle;
    bool bIsBound = false;

#pragma region Inventory UI
public:
    UPROPERTY(meta = (BindWidget))
    UImage* SlotIcon_0;

    UPROPERTY(meta = (BindWidget))
    UImage* SlotIcon_1;

    UPROPERTY(meta = (BindWidget))
    UImage* SlotIcon_2;

    UPROPERTY(meta = (BindWidget))
    UImage* SlotIcon_3;

    UPROPERTY(meta = (BindWidget))
    UImage* SlotIcon_4;

    UPROPERTY()
    TArray<TObjectPtr<UImage>> SlotIcons;

    void Init(ASurvivorPlayerState* PS);

    UFUNCTION()
    void RefreshInventory();

#pragma endregion
};