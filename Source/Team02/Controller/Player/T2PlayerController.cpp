#include "Controller/Player/T2PlayerController.h"
#include "UW_SurvivorHUD.h"

void AT2PlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (IsLocalPlayerController())
    {
        ShowSurvivorHUD();
    }
}

void AT2PlayerController::ShowSurvivorHUD()
{
    if (SurvivorHUDWidgetClass)
    {
        SurvivorHUDWidgetInstance = CreateWidget<UUW_SurvivorHUD>(this, SurvivorHUDWidgetClass);

        if (SurvivorHUDWidgetInstance)
        {
            SurvivorHUDWidgetInstance->AddToViewport();
        }
    }
}


void AT2PlayerController::StartSpectate(AActor* Target)
{
    SetIgnoreMoveInput(true);
    SetIgnoreLookInput(true);
    ChangeState(NAME_Spectating);
    SetViewTargetWithBlend(Target, 0.5f);
    ShowSpectatorUI();
}

void AT2PlayerController::NextSpectate()
{
}

void AT2PlayerController::ShowSpectatorUI()
{
}