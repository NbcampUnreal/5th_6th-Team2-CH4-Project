// Fill out your copyright notice in the Description page of Project Settings.

#include "Controller/Killer/T2KillerController.h"

#include "Blueprint/UserWidget.h"
#include "UI/UW_KillerHUD.h"

/*
 * ========================================
 * 컨트롤러 통합 작업 중 - 주석 처리됨
 * 역할: AT2BaseController로 통합 예정
 * ========================================
 */

void AT2KillerController::BeginPlay()
{
	Super::BeginPlay();

	/*
	if (IsLocalPlayerController())
	{
		ShowKillerHUD();
	}
	*/
}

void AT2KillerController::ShowKillerHUD()
{
	/*
	if (KillerHUDWidgetClass)
	{
		KillerHUDWidgetInstance = CreateWidget<UUW_KillerHUD>(this, KillerHUDWidgetClass);

		if (KillerHUDWidgetInstance)
		{
			KillerHUDWidgetInstance->AddToViewport();
		}
	}
	*/
}
