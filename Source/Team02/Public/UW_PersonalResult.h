#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_PersonalResult.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class TEAM02_API UUW_PersonalResult : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// 결과 설정 (탈출 여부)
	UFUNCTION(BlueprintCallable, Category = "Result")
	void SetResult(bool bEscaped);

	// 매치 종료 결과 표시 (승/패)
	UFUNCTION(BlueprintCallable, Category = "Result")
	void ShowMatchResult(bool bSurvivorWin);

protected:
	// 결과 텍스트 (예: "탈출 성공!" / "사망...")
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ResultText;

	// 매치 결과 텍스트 (예: "생존자 승리!" / "살인마 승리...")
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* MatchResultText;

	// 관전하기 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* SpectateButton;

	// 나가기 버튼
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* LeaveButton;

private:
	UFUNCTION()
	void OnSpectateButtonClicked();

	UFUNCTION()
	void OnLeaveButtonClicked();

	// 관전 가능 여부 확인 및 버튼 상태 업데이트
	void UpdateSpectateButtonState();

	// 살아있는 생존자 있는지 확인
	bool HasAliveTargets() const;
};
