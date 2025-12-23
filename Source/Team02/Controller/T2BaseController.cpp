#include "Controller/T2BaseController.h"
#include "PlayerState/T2PlayerState.h"
#include "UI/UW_KillerHUD.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

AT2BaseController::AT2BaseController()
{
}

void AT2BaseController::BeginPlay()
{
    Super::BeginPlay();

    // 로컬 플레이어만 HUD 처리
    if (!IsLocalPlayerController())
    {
        return;
    }

    // PlayerState가 이미 있으면 Role 확인
    if (AT2PlayerState* PS = GetPlayerState<AT2PlayerState>())
    {
        if (PS->PlayerRole != EPlayerRole::None)
        {
            UpdateHUDForRole(PS->PlayerRole);
        }

        // Role 변경 델리게이트 바인딩
        PS->OnPlayerRoleChanged.AddDynamic(this, &AT2BaseController::OnPlayerRoleChanged);
    }
}

void AT2BaseController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    // 클라이언트에서 PlayerState가 복제되었을 때
    if (!IsLocalPlayerController())
    {
        return;
    }

    if (AT2PlayerState* PS = GetPlayerState<AT2PlayerState>())
    {
        // 델리게이트 바인딩 (아직 안 되어있다면)
        if (!PS->OnPlayerRoleChanged.IsAlreadyBound(this, &AT2BaseController::OnPlayerRoleChanged))
        {
            PS->OnPlayerRoleChanged.AddDynamic(this, &AT2BaseController::OnPlayerRoleChanged);
        }

        // Role이 이미 설정되어 있으면 HUD 업데이트
        if (PS->PlayerRole != EPlayerRole::None)
        {
            UpdateHUDForRole(PS->PlayerRole);
        }
    }
}

void AT2BaseController::OnPlayerRoleChanged(EPlayerRole NewRole)
{
    if (IsLocalPlayerController())
    {
        UpdateHUDForRole(NewRole);
    }
}

void AT2BaseController::UpdateHUDForRole(EPlayerRole NewRole)
{
    // 이미 같은 Role이면 스킵
    if (CurrentDisplayedRole == NewRole)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("UpdateHUDForRole: %s"),
        NewRole == EPlayerRole::Killer ? TEXT("Killer") :
        NewRole == EPlayerRole::Survivor ? TEXT("Survivor") : TEXT("None"));

    HideAllHUD();

    CurrentDisplayedRole = NewRole;

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

    if (KillerHUDInstance && !KillerHUDInstance->IsInViewport())
    {
        KillerHUDInstance->AddToViewport();
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

    if (SurvivorHUDInstance && !SurvivorHUDInstance->IsInViewport())
    {
        SurvivorHUDInstance->AddToViewport();
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

void AT2BaseController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // ESC 키 바인딩 (Enhanced Input 사용 시)
    // 필요하다면 여기에 IA_Settings 액션 바인딩 추가
}

void AT2BaseController::ToggleSettingsMenu()
{
    if (!SettingsMenuWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("SettingsMenuWidgetClass is not set!"));
        return;
    }

    if (bIsSettingsMenuOpen)
    {
        // 설정창 닫기
        if (SettingsMenuInstance)
        {
            SettingsMenuInstance->RemoveFromParent();
        }
        bIsSettingsMenuOpen = false;

        // 게임 입력 모드로 복귀
        FInputModeGameOnly InputMode;
        SetInputMode(InputMode);
        bShowMouseCursor = false;
    }
    else
    {
        // 설정창 열기
        if (!SettingsMenuInstance)
        {
            SettingsMenuInstance = CreateWidget<UUserWidget>(this, SettingsMenuWidgetClass);
        }

        if (SettingsMenuInstance)
        {
            SettingsMenuInstance->AddToViewport(100); // 높은 Z-Order
        }
        bIsSettingsMenuOpen = true;

        // UI 입력 모드
        FInputModeGameAndUI InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        SetInputMode(InputMode);
        bShowMouseCursor = true;
    }
}