// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/Killer/T2KillerController.h"

#include "Blueprint/UserWidget.h"
#include "UI/UW_KillerHUD.h"

void AT2KillerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		ShowKillerHUD();
	}
}

void AT2KillerController::ShowKillerHUD()
{
	if (KillerHUDWidgetClass)
	{
		KillerHUDWidgetInstance = CreateWidget<UUW_KillerHUD>(this, KillerHUDWidgetClass);

		if (KillerHUDWidgetInstance)
		{
			KillerHUDWidgetInstance->AddToViewport();
		}
	}
}