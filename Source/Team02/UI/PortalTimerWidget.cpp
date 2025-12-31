// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PortalTimerWidget.h"
#include "Gimmick/Portal/PortalActor.h"
#include "Kismet/GameplayStatics.h"
#include "T2PlayGameState.h"

void UPortalTimerWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPortalTimerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

FText UPortalTimerWidget::GetPortalTimeText() const
{
	APortalActor* Portal = FindActivePortal();
	if (!Portal || !Portal->IsPortalActive())
	{
		return FText::FromString(TEXT("--:--"));
	}

	float RemainingTime = Portal->GetRemainingTime();
	int32 Minutes = FMath::FloorToInt(RemainingTime / 60.0f);
	int32 Seconds = FMath::FloorToInt(RemainingTime) % 60;

	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds));
}

FText UPortalTimerWidget::GetPlayersEnteredText() const
{
	APortalActor* Portal = FindActivePortal();
	AT2PlayGameState* GS = GetWorld()->GetGameState<AT2PlayGameState>();
	
	if (!Portal || !GS)
	{
		return FText::FromString(TEXT("0/0"));
	}

	int32 EnteredCount = Portal->GetEnteredPlayersCount();
	int32 TotalAlive = GS->GetAliveSurvivorCount() + EnteredCount; // 들어간 사람도 포함

	return FText::FromString(FString::Printf(TEXT("%d/%d"), EnteredCount, TotalAlive));
}

bool UPortalTimerWidget::IsPortalActive() const
{
	APortalActor* Portal = FindActivePortal();
	return Portal && Portal->IsPortalActive();
}

APortalActor* UPortalTimerWidget::FindActivePortal() const
{
	TArray<AActor*> FoundPortals;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APortalActor::StaticClass(), FoundPortals);

	for (AActor* Actor : FoundPortals)
	{
		APortalActor* Portal = Cast<APortalActor>(Actor);
		if (Portal && Portal->IsPortalActive())
		{
			return Portal;
		}
	}

	return nullptr;
}