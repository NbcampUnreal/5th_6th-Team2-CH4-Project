#include "UW_SurvivorHUD.h"
#include "Components/ProgressBar.h"
#include "PlayerState/Player/SurvivorPlayerState.h"
#include "PlayerState/Player/InventoryComponent.h"
#include "Components/Image.h"
#include "Public/ItemData.h"
#include "Gimmick/Player/FlashlightComponent.h"

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

    if (!bIsBound)
    {
        TryBindToPlayerState();
    }
}

void UUW_SurvivorHUD::TryBindToPlayerState()
{
    if (bIsBound) return;

    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;

    ASurvivorPlayerState* PS = PC->GetPlayerState<ASurvivorPlayerState>();
    if (!PS) return;
 
    APawn* Pawn = Cast<APawn>(PC->GetPawn());
    if (!Pawn) return;
  
    UFlashlightComponent* FL = Cast<UFlashlightComponent>(Pawn->FindComponentByClass<UFlashlightComponent>());
   
    if (!FL) return;

    HPChangedHandle = PS->OnHPChanged.AddUObject(this, &UUW_SurvivorHUD::UpdateHP);
    BatteryChangedHandle = FL->OnBatteryChanged.AddUObject(this, &UUW_SurvivorHUD::UpdateBattery);
    bIsBound = true;

    UpdateHP(PS->CurrentHP, PS->MaxHP);
    UpdateBattery(FL->CurrentBattery, FL->MaxBattery);

    UE_LOG(LogTemp, Warning, TEXT("HP Binding SUCCESS"));
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
    }
    APawn* Pawn = Cast<APawn>(PC->GetPawn());
    if (IsValid(Pawn) == true)
    {
        UFlashlightComponent* FL = Cast<UFlashlightComponent>(Pawn->FindComponentByClass<UFlashlightComponent>());
        if (IsValid(FL) == true)
        {
            FL->OnBatteryChanged.Remove(BatteryChangedHandle);
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