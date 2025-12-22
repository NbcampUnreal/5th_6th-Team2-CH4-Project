#include "Controller/Player/T2PlayerController.h"
#include "UW_SurvivorHUD.h"
#include "T2PlayGameMod.h"
#include "UI/UW_RoundProgressBar.h"
#include "Components/Image.h"

void AT2PlayerController::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Error, TEXT("=== T2PlayerController BeginPlay ==="));
    UE_LOG(LogTemp, Error, TEXT("IsLocal:  %s, HasAuthority: %s"),
        IsLocalPlayerController() ? TEXT("YES") : TEXT("NO"),
        HasAuthority() ? TEXT("YES") : TEXT("NO"));

    if (IsLocalPlayerController())
    {
        UE_LOG(LogTemp, Error, TEXT("Creating HUD..."));
        ShowSurvivorHUD();
    }
}

void AT2PlayerController::ShowSurvivorHUD()
{
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

void AT2PlayerController::StartInteractUI()
{
    if (bInteractUIActive) return;
    if (!InteractWidgetClass) return;

    InteractWidgetClassInstance = CreateWidget<UUW_RoundProgressBar>(this, InteractWidgetClass);

    if (!InteractWidgetClassInstance) return;

    InteractWidgetClassInstance->AddToViewport();
    bInteractUIActive = true;

    UImage* ProgressImage = Cast<UImage>(InteractWidgetClassInstance->GetWidgetFromName(TEXT("RoundPrgressImage")));

    if (IsValid(ProgressImage) == true)
    {
        InteractMID = ProgressImage->GetDynamicMaterial();
        InteractMID->SetScalarParameterValue(TEXT("Percent"), 0.f);
    }

}

void AT2PlayerController::UpdateInteractUI(float Percent)
{
    if (!bInteractUIActive || !InteractMID) return;

    Percent = FMath::Clamp(Percent, 0.f, 1.f);
    InteractMID->SetScalarParameterValue(TEXT("Percent"), Percent);
}

void AT2PlayerController::StopInteractUI()
{
    if (!bInteractUIActive) return;

    if (InteractWidgetClassInstance)
    {
        InteractWidgetClassInstance->RemoveFromParent();
        InteractWidgetClassInstance = nullptr;
    }

    InteractMID = nullptr;
    bInteractUIActive = false;
}
