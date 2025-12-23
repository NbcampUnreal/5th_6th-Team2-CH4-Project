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

    // 클라이언트:  PlayerState가 복제되면 Role 확인하여 HUD 업데이트
    if (AT2PlayerState* PS = GetPlayerState<AT2PlayerState>())
    {
        UE_LOG(LogTemp, Warning, TEXT("OnRep_PlayerState: Role = %d"), (int32)PS->PlayerRole);

        if (PS->PlayerRole != EPlayerRole::None)
        {
            UpdateHUDForRole(PS->PlayerRole);
        }
    }
}

// ★ 추가
void AT2BaseController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // 서버:  Possess 시점에 HUD 업데이트
    if (IsLocalPlayerController())
    {
        if (AT2PlayerState* PS = GetPlayerState<AT2PlayerState>())
        {
            UE_LOG(LogTemp, Warning, TEXT("OnPossess:  Role = %d"), (int32)PS->PlayerRole);

            if (PS->PlayerRole != EPlayerRole::None)
            {
                UpdateHUDForRole(PS->PlayerRole);
            }
        }
    }
}

void AT2BaseController::SetupInputComponent()
{
    Super::SetupInputComponent();
}

void AT2BaseController::UpdateHUDForRole(EPlayerRole NewRole)
{
    if (!IsLocalPlayerController())
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateHUDForRole: Not local controller, skipping HUD"));
        return;
    }

    if (NewRole == EPlayerRole::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateHUDForRole: Role is None, skipping"));
        return;
    }

    // 이미 같은 Role로 HUD 표시했으면 스킵
    if (bHUDInitialized && CurrentDisplayedRole == NewRole)
    {
        return;
    }

    CurrentDisplayedRole = NewRole;
    bHUDInitialized = true;

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
        UE_LOG(LogTemp, Warning, TEXT("KillerHUDWidgetClass is not set! "));
        return;
    }

    if (!KillerHUDInstance)
    {
        KillerHUDInstance = CreateWidget<UUW_KillerHUD>(this, KillerHUDWidgetClass);
    }

    if (KillerHUDInstance)
    {
        KillerHUDInstance->AddToViewport();
        UE_LOG(LogTemp, Warning, TEXT("KillerHUD displayed! "));
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
}

void AT2BaseController::StartInteractUI()
{
    bInteractUIActive = true;
}

void AT2BaseController::UpdateInteractUI(float Percent)
{
}

void AT2BaseController::StopInteractUI()
{
    bInteractUIActive = false;
}