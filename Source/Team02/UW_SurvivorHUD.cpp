#include "UW_SurvivorHUD.h"
#include "Components/ProgressBar.h"
#include "PlayerState/Player/SurvivorPlayerState.h"

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

    if (Item)
    {
        Item->SetVisibility(ESlateVisibility::Hidden);
    }

    TryBindToPlayerState();
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

    HPChangedHandle = PS->OnHPChanged.AddUObject(this, &UUW_SurvivorHUD::UpdateHP);
    bIsBound = true;

    UpdateHP(PS->CurrentHP, PS->MaxHP);

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

void UUW_SurvivorHUD::ShowItemBar()
{
    if (Item)
    {
        Item->SetVisibility(ESlateVisibility::Visible);
    }
}

void UUW_SurvivorHUD::HideItemBar()
{
    if (Item)
    {
        Item->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UUW_SurvivorHUD::SetItemPercent(float Percent)
{
    if (Item)
    {
        Item->SetPercent(FMath::Clamp(Percent, 0.f, 1.f));
    }
}