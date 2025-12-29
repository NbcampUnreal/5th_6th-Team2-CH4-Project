#include "UI/UW_KillerHUD.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "T2PlayGameState.h"

void UUW_KillerHUD::NativeConstruct()
{
	Super::NativeConstruct();

	// 포탈 타이머 초기 숨김
	if (PortalTimerText)
	{
		PortalTimerText->SetVisibility(ESlateVisibility::Hidden);
	}
	if (PortalTimerPanel)
	{
		PortalTimerPanel->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UUW_KillerHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdatePortalTimer();
}

void UUW_KillerHUD::UpdatePortalTimer()
{
	AT2PlayGameState* GS = GetWorld()->GetGameState<AT2PlayGameState>();
	if (!GS) return;

	bool bPortalActive = GS->IsPortalActive();

	// 포탈이 활성화되지 않았으면 숨김
	if (!bPortalActive)
	{
		if (PortalTimerText)
		{
			PortalTimerText->SetVisibility(ESlateVisibility::Hidden);
		}
		if (PortalTimerPanel)
		{
			PortalTimerPanel->SetVisibility(ESlateVisibility::Hidden);
		}
		return;
	}

	// 포탈 활성화 시 표시
	if (PortalTimerText)
	{
		PortalTimerText->SetVisibility(ESlateVisibility::Visible);
	}
	if (PortalTimerPanel)
	{
		PortalTimerPanel->SetVisibility(ESlateVisibility::Visible);
	}

	// 남은 시간 표시
	float RemainingTime = GS->GetPortalRemainingTime();
	int32 Minutes = FMath::FloorToInt(RemainingTime / 60.0f);
	int32 Seconds = FMath::FloorToInt(RemainingTime) % 60;

	FString TimeString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
	
	if (PortalTimerText)
	{
		PortalTimerText->SetText(FText::FromString(TimeString));
	}
}
