#include "Controller/Player/T2PlayerController.h"
#include "UW_SurvivorHUD.h"
#include "T2PlayGameMod.h"

/*
 * ========================================
 * 컨트롤러 통합 작업 중 - 주석 처리됨
 * 역할: AT2BaseController로 통합 예정
 * ========================================
 */

void AT2PlayerController::BeginPlay()
{
    Super::BeginPlay();

    /*
    UE_LOG(LogTemp, Error, TEXT("=== T2PlayerController BeginPlay ==="));
    UE_LOG(LogTemp, Error, TEXT("IsLocal:  %s, HasAuthority: %s"),
        IsLocalPlayerController() ? TEXT("YES") : TEXT("NO"),
        HasAuthority() ? TEXT("YES") : TEXT("NO"));

    if (IsLocalPlayerController())
    {
        UE_LOG(LogTemp, Error, TEXT("Creating HUD..."));
        ShowSurvivorHUD();
    }
    */
}

void AT2PlayerController::ShowSurvivorHUD()
{
    /*
    UE_LOG(LogTemp, Error, TEXT("=== ShowSurvivorHUD ==="));

    if (!SurvivorHUDWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("SurvivorHUDWidgetClass is NULL - SET IT IN BP"));
        return;
    }

    UE_LOG(LogTemp, Error, TEXT("Creating Widget..."));

    SurvivorHUDWidgetInstance = CreateWidget<UUW_SurvivorHUD>(this, SurvivorHUDWidgetClass);

    if (SurvivorHUDWidgetInstance)
    {
        SurvivorHUDWidgetInstance->AddToViewport();
        UE_LOG(LogTemp, Error, TEXT("SurvivorHUD Added to Viewport SUCCESS"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create widget"));
    }
    */
}

void AT2PlayerController::StartSpectate(AActor* Target)
{
    /*
    SetIgnoreMoveInput(true);
    SetIgnoreLookInput(true);
    ChangeState(NAME_Spectating);
    SetViewTargetWithBlend(Target, 0.5f);
    ShowSpectatorUI();
    */
}

void AT2PlayerController::NextSpectate()
{
}

void AT2PlayerController::ShowSpectatorUI()
{
}
