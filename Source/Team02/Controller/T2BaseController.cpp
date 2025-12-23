#include "Controller/T2BaseController.h"
#include "PlayerState/T2PlayerState.h"
#include "UI/UW_KillerHUD.h"
#include "Blueprint/UserWidget.h"

AT2BaseController::AT2BaseController()
{
}

void AT2BaseController::BeginPlay()
{
    Super::BeginPlay();
}

void AT2BaseController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    // PlayerState가 복제되면 Role 확인하여 HUD 업데이트
    if (AT2PlayerState* PS = GetPlayerState<AT2PlayerState>())
    {
        UpdateHUDForRole(PS->PlayerRole);
    }
}

void AT2BaseController::SetupInputComponent()
{
    Super::SetupInputComponent();
}

void AT2BaseController::UpdateHUDForRole(EPlayerRole NewRole)
{
    // ★★★ 로컬 플레이어만 HUD 표시 ★★★
    if (!IsLocalPlayerController())
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateHUDForRole: Not local controller, skipping HUD"));
        return;
    }

    // 이미 같은 Role이면 스킵
    if (CurrentDisplayedRole == NewRole)
    {
        return;
    }

    CurrentDisplayedRole = NewRole;

    UE_LOG(LogTemp, Warning, TEXT("UpdateHUDForRole: %s"),
        NewRole == EPlayerRole::Killer ? TEXT("Killer") :
        NewRole == EPlayerRole::Survivor ? TEXT("Survivor") : TEXT("None"));

    // 기존 HUD 숨기기
    HideAllHUD();

    // 새 Role에 맞는 HUD 표시
    if (NewRole == EPlayerRole::Killer)
    {
        ShowKillerHUD();
    }
    else if (NewRole == EPlayerRole::Survivor)
    {
        ShowSurvivorHUD();
    }
}

void AT2BaseController::ShowKillerHUD()
{
    if (!KillerHUDWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("KillerHUDWidgetClass is not set!"));
        return;
    }

    if (!KillerHUDInstance)
    {
        KillerHUDInstance = CreateWidget<UUW_KillerHUD>(this, KillerHUDWidgetClass);
    }

    if (KillerHUDInstance)
    {
        KillerHUDInstance->AddToViewport();
        UE_LOG(LogTemp, Warning, TEXT("KillerHUD displayed!"));
    }
}

void AT2BaseController::ShowSurvivorHUD()
{
    if (!SurvivorHUDWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("SurvivorHUDWidgetClass is not set!"));
        return;
    }

    if (!SurvivorHUDInstance)
    {
        SurvivorHUDInstance = CreateWidget<UUserWidget>(this, SurvivorHUDWidgetClass);
    }

    if (SurvivorHUDInstance)
    {
        SurvivorHUDInstance->AddToViewport();
        UE_LOG(LogTemp, Warning, TEXT("SurvivorHUD displayed!"));
    }
}

void AT2BaseController::HideAllHUD()
{
    if (KillerHUDInstance && KillerHUDInstance->IsInViewport())
    {
        KillerHUDInstance->RemoveFromParent();
    }

    if (SurvivorHUDInstance && SurvivorHUDInstance->IsInViewport())
    {
        SurvivorHUDInstance->RemoveFromParent();
    }
}

void AT2BaseController::OnPlayerRoleChanged(EPlayerRole NewRole)
{
    UpdateHUDForRole(NewRole);
}

void AT2BaseController::ToggleSettingsMenu()
{
    // ESC 설정창 - 나중에 구현
}

void AT2BaseController::StartInteractUI()
{
    // InteractUI - 나중에 구현
    bInteractUIActive = true;
}

void AT2BaseController::UpdateInteractUI(float Percent)
{
    // InteractUI 업데이트 - 나중에 구현
}

void AT2BaseController::StopInteractUI()
{
    // InteractUI 종료 - 나중에 구현
    bInteractUIActive = false;
}