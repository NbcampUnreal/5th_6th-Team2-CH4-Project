// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Controller/T2BaseController.h"
#include "T2KillerController.generated.h"

/*
 * ========================================
 * 컨트롤러 통합 작업 중 - 주석 처리됨
 * 역할: AT2BaseController로 통합 예정
 * ========================================
 */

class UUW_KillerHUD;

UCLASS()
class TEAM02_API AT2KillerController : public AT2BaseController
{
	GENERATED_BODY()

protected:
	/*
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUW_KillerHUD> KillerHUDWidgetClass; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UUW_KillerHUD> KillerHUDWidgetInstance; 
	*/

	virtual void BeginPlay() override;

private:
	void ShowKillerHUD();
};
