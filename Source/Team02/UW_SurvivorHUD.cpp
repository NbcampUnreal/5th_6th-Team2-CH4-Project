#include "UW_SurvivorHUD.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "PlayerState/Player/SurvivorPlayerState.h"
#include "PlayerState/Player/InventoryComponent.h"
#include "Components/Image.h"
#include "Public/ItemData.h"
#include "Gimmick/Player/FlashlightComponent.h"
#include "T2PlayGameState.h"

void UUW_SurvivorHUD::NativeConstruct()
{
    Super::NativeConstruct();

    UE_LOG(LogTemp, Warning, TEXT("UW_SurvivorHUD NativeConstruct"));

    if (HP)
    {
        HP->SetFillColorAndOpacity(FLinearColor(0.9f, 0.0f, 0.0f, 1.f));
        HP->SetPercent(1.0f);
        UE_LOG(LogTemp, Warning, TEXT("HP ProgressBar found"));
    }

    if (Battery)
    {
        Battery->SetPercent(1.0f);
    }

    // 키 카운트 초기화
    if (KeyCountText)
    {
        KeyCountText->SetText(FText::FromString(TEXT("0 / 6")));
    }

    // 포탈 타이머 초기 숨김
    if (PortalTimerText)
    {
        PortalTimerText->SetVisibility(ESlateVisibility::Hidden);
    }
    if (PortalTimerPanel)
    {
        PortalTimerPanel->SetVisibility(ESlateVisibility::Hidden);
    }

    TryBindToPlayerState();

    SlotIcons.Empty();
    SlotIcons.Add(SlotIcon_0);
    SlotIcons.Add(SlotIcon_1);
    SlotIcons.Add(SlotIcon_2);
    SlotIcons.Add(SlotIcon_3);
    SlotIcons.Add(SlotIcon_4);
}

void UUW_SurvivorHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!bIsHPBound || !bIsBatteryBound)
    {
        TryBindToPlayerState();
    }

    // 매 틱마다 키 카운트와 포탈 타이머 업데이트
    UpdateKeyCount();
    UpdatePortalTimer();
}

void UUW_SurvivorHUD::TryBindToPlayerState()
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;

    // HP 바인딩 (PlayerState만 있으면 됨)
    if (!bIsHPBound)
    {
        ASurvivorPlayerState* PS = PC->GetPlayerState<ASurvivorPlayerState>();
        if (PS)
        {
            HPChangedHandle = PS->OnHPChanged.AddUObject(this, &UUW_SurvivorHUD::UpdateHP);
            bIsHPBound = true;
            UpdateHP(PS->CurrentHP, PS->MaxHP);
            UE_LOG(LogTemp, Warning, TEXT("HP Binding SUCCESS"));
        }
    }

    // Battery 바인딩 (Pawn + FlashlightComponent 필요)
    if (!bIsBatteryBound)
    {
        APawn* Pawn = PC->GetPawn();
        if (Pawn)
        {
            UFlashlightComponent* FL = Pawn->FindComponentByClass<UFlashlightComponent>();
            if (FL)
            {
                BatteryChangedHandle = FL->OnBatteryChanged.AddUObject(this, &UUW_SurvivorHUD::UpdateBattery);
                bIsBatteryBound = true;
                UpdateBattery(FL->CurrentBattery, FL->MaxBattery);
                UE_LOG(LogTemp, Warning, TEXT("Battery Binding SUCCESS"));
            }
        }
    }

    bIsBound = bIsHPBound && bIsBatteryBound;
}

void UUW_SurvivorHUD::NativeDestruct()
{
    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        if (ASurvivorPlayerState* PS = PC->GetPlayerState<ASurvivorPlayerState>())
        {
            PS->OnHPChanged.Remove(HPChangedHandle);
        }

        APawn* Pawn = PC->GetPawn();
        if (Pawn)
        {
            UFlashlightComponent* FL = Pawn->FindComponentByClass<UFlashlightComponent>();
            if (FL)
            {
                FL->OnBatteryChanged.Remove(BatteryChangedHandle);
            }
        }
    }

    Super::NativeDestruct();
}

void UUW_SurvivorHUD::UpdateHP(float CurrentHP, float MaxHP)
{
    UE_LOG(LogTemp, Error, TEXT("=== UpdateHP Called === Current:  %f, Max: %f"), CurrentHP, MaxHP);

    if (HP && MaxHP > 0.f)
    {
        float Percent = CurrentHP / MaxHP;
        HP->SetPercent(Percent);
        UE_LOG(LogTemp, Error, TEXT("HP Bar Percent set to: %f"), Percent);
    }
}

void UUW_SurvivorHUD::UpdateBattery(float CurrentBattery, float MaxBattery)
{
    if (Battery && MaxBattery > 0.f)
    {
        float Percent = CurrentBattery / MaxBattery;
        Battery->SetPercent(Percent);
    }
}

void UUW_SurvivorHUD::UpdateKeyCount()
{
    if (!KeyCountText) return;

    AT2PlayGameState* GS = GetWorld()->GetGameState<AT2PlayGameState>();
    if (!GS) return;

    int32 CurrentKeys = GS->GetKeyCount();
    int32 RequiredKeys = GS->GetRequiredKeys();

    FString KeyString = FString::Printf(TEXT("%d / %d"), CurrentKeys, RequiredKeys);
    KeyCountText->SetText(FText::FromString(KeyString));
}

void UUW_SurvivorHUD::UpdatePortalTimer()
{
    AT2PlayGameState* GS = GetWorld()->GetGameState<AT2PlayGameState>();
    if (!GS) return;

    bool bPortalActive = GS->IsPortalActive();

    // 포탈이 활성화되지 않았으면 숨김
    if (!bPortalActive)
    {
        if (PortalTimerText)
        {
            PortalTimerText->SetVisibility(ESlateVisibility::Hidden);
        }
        if (PortalTimerPanel)
        {
            PortalTimerPanel->SetVisibility(ESlateVisibility::Hidden);
        }
        return;
    }

    // 포탈 활성화 시 표시
    if (PortalTimerText)
    {
        PortalTimerText->SetVisibility(ESlateVisibility::Visible);
    }
    if (PortalTimerPanel)
    {
        PortalTimerPanel->SetVisibility(ESlateVisibility::Visible);
    }

    // 남은 시간 표시
    float RemainingTime = GS->GetPortalRemainingTime();
    int32 Minutes = FMath::FloorToInt(RemainingTime / 60.0f);
    int32 Seconds = FMath::FloorToInt(RemainingTime) % 60;

    FString TimeString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
    
    if (PortalTimerText)
    {
        PortalTimerText->SetText(FText::FromString(TimeString));
    }
}

void UUW_SurvivorHUD::ShowBatteryBar()
{
    if (Battery)
    {
        Battery->SetVisibility(ESlateVisibility::Visible);
    }
}

void UUW_SurvivorHUD::HideBatteryBar()
{
    if (Battery)
    {
        Battery->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UUW_SurvivorHUD::SetBatteryPercent(float Percent)
{
    if (Battery)
    {
        Battery->SetPercent(FMath::Clamp(Percent, 0.f, 1.f));
    }
}

void UUW_SurvivorHUD::Init(ASurvivorPlayerState* PS)
{
    if (!PS) return;

    if (PS->InventoryComponent)
    {
        PS->InventoryComponent->OnInventoryUpdated.AddDynamic(this, &UUW_SurvivorHUD::RefreshInventory);
        RefreshInventory();
    }
}

void UUW_SurvivorHUD::RefreshInventory()
{
    ASurvivorPlayerState* PS = GetOwningPlayerState<ASurvivorPlayerState>();
    if (!PS) return;

    const TArray<FInventorySlot>& Items = PS->InventoryComponent->Items;

    for (int32 i = 0; i < SlotIcons.Num(); i++)
    {
        if (!Items.IsValidIndex(i) || Items[i].ItemID == NAME_None)
        {
            SlotIcons[i]->SetVisibility(ESlateVisibility::Hidden);
            continue;
        }

        UDataTable* DT = PS->InventoryComponent->ItemDataTable;
        if (!DT) return;

        const FName RowName = Items[i].ItemID;
        const FItemData* ItemData = DT->FindRow<FItemData>(RowName, TEXT("InventoryUI"));

        if (ItemData && ItemData->Icon)
        {
            SlotIcons[i]->SetBrushFromTexture(ItemData->Icon);
            SlotIcons[i]->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            SlotIcons[i]->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}
