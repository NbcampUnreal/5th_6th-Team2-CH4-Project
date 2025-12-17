#include "UW_SurvivorHUD.h"
#include "Components/ProgressBar.h"
#include "PlayerState/Player/SurvivorPlayerState.h"

void UUW_SurvivorHUD::NativeConstruct()
{
    Super::NativeConstruct();


    if (Item)
    {
        Item->SetVisibility(ESlateVisibility::Hidden);
    }


    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;

    ASurvivorPlayerState* PS = PC->GetPlayerState<ASurvivorPlayerState>();
    if (!PS) return;


    HPChangedHandle = PS->OnHPChanged.AddUObject(this, &UUW_SurvivorHUD::UpdateHP);


    UpdateHP(PS->CurrentHP, PS->MaxHP);
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
    if (HP && MaxHP > 0.f)
    {
        HP->SetPercent(CurrentHP / MaxHP);
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