#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_KillerHUD.generated.h"

class UT2CooldownComponent;
class UTextBlock;
class UCanvasPanel;

UCLASS()
class TEAM02_API UUW_KillerHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	// ★ 포탈 타이머 관련 (평소엔 숨김)
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PortalTimerText;

	UPROPERTY(meta = (BindWidgetOptional))
	UCanvasPanel* PortalTimerPanel;  // 포탈 타이머 전체를 감싸는 패널 (숨김/표시용)

private:
	// ★ 포탈 타이머 업데이트
	void UpdatePortalTimer();
};
