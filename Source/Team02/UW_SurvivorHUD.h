#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_SurvivorHUD.generated.h"

class UImage;
class UTextBlock;
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

    // ★ 키 카운트 텍스트 (예: "3 / 6")
    UPROPERTY(meta = (BindWidget))
    UTextBlock* KeyCountText;

    // ★ 포탈 타이머 관련 (평소엔 숨김)
    UPROPERTY(meta = (BindWidget))
    UTextBlock* PortalTimerText;

    UPROPERTY(meta = (BindWidgetOptional))
    class UCanvasPanel* PortalTimerPanel;  // 포탈 타이머 전체를 감싸는 패널 (숨김/표시용)

private:
    UFUNCTION()
    void UpdateHP(float CurrentHP, float MaxHP);

    UFUNCTION()
    void UpdateBattery(float CurrentBattery, float MaxBattery);

    void TryBindToPlayerState();

    // ★ 키 카운트 업데이트
    void UpdateKeyCount();

    // ★ 포탈 타이머 업데이트
    void UpdatePortalTimer();

    FDelegateHandle HPChangedHandle;
    FDelegateHandle BatteryChangedHandle;

    bool bIsBound = false;
    bool bIsHPBound = false;
    bool bIsBatteryBound = false;

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
