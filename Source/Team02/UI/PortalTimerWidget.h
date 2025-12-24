// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PortalTimerWidget.generated.h"

UCLASS()
class TEAM02_API UPortalTimerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UFUNCTION(BlueprintCallable, Category = "Portal")
	FText GetPortalTimeText() const;

	UFUNCTION(BlueprintCallable, Category = "Portal")
	FText GetPlayersEnteredText() const;

	UFUNCTION(BlueprintCallable, Category = "Portal")
	bool IsPortalActive() const;

private:
	class APortalActor* FindActivePortal() const;
};