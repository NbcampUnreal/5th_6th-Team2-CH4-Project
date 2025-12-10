// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Controller/T2BaseController.h"
#include "T2KillerController.generated.h"

class UUW_KillerHUD;

/**
 * 
 */
UCLASS()
class TEAM02_API AT2KillerController : public AT2BaseController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUW_KillerHUD> KillerHUDWidgetClass; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UUW_KillerHUD> KillerHUDWidgetInstance; 

	virtual void BeginPlay() override;

private:
	void ShowKillerHUD();
};
