#include "./Public/UW_PersonalResult.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Controller/T2BaseController.h"
#include "T2PlayGameState.h"
#include "PlayerState/Player/SurvivorPlayerState.h"

void UUW_PersonalResult::NativeConstruct()
{
	Super::NativeConstruct();

	if (SpectateButton)
	{
		SpectateButton->OnClicked.AddDynamic(this, &UUW_PersonalResult::OnSpectateButtonClicked);
	}

	if (LeaveButton)
	{
		LeaveButton->OnClicked.AddDynamic(this, &UUW_PersonalResult::OnLeaveButtonClicked);
	}

	if (MatchResultText)
	{
		MatchResultText->SetVisibility(ESlateVisibility::Hidden);
	}

	UpdateSpectateButtonState();
}

void UUW_PersonalResult::SetResult(bool bEscaped)
{
	if (!ResultText) return;

	if (bEscaped)
	{
		ResultText->SetText(FText::FromString(TEXT("ESCAPED!")));
		ResultText->SetColorAndOpacity(FSlateColor(FLinearColor(0.0f, 1.0f, 0.3f, 1.0f)));
	}
	else
	{
		ResultText->SetText(FText::FromString(TEXT("DEAD...")));
		ResultText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.2f, 0.2f, 1.0f)));
	}

	UpdateSpectateButtonState();
}

void UUW_PersonalResult::ShowMatchResult(bool bSurvivorWin)
{
	if (!MatchResultText) return;

	MatchResultText->SetVisibility(ESlateVisibility::Visible);

	if (bSurvivorWin)
	{
		MatchResultText->SetText(FText::FromString(TEXT("SURVIVORS WIN!")));
		MatchResultText->SetColorAndOpacity(FSlateColor(FLinearColor(0.0f, 1.0f, 0.3f, 1.0f)));
	}
	else
	{
		MatchResultText->SetText(FText::FromString(TEXT("KILLER WINS!")));
		MatchResultText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.2f, 0.2f, 1.0f)));
	}

	if (SpectateButton)
	{
		SpectateButton->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UUW_PersonalResult::OnSpectateButtonClicked()
{
	if (AT2BaseController* PC = Cast<AT2BaseController>(GetOwningPlayer()))
	{
		PC->StartSpectating();
	}
}

void UUW_PersonalResult::OnLeaveButtonClicked()
{
	if (AT2BaseController* PC = Cast<AT2BaseController>(GetOwningPlayer()))
	{
		PC->RequestLeaveGame();
	}
}

void UUW_PersonalResult::UpdateSpectateButtonState()
{
	if (!SpectateButton) return;

	if (HasAliveTargets())
	{
		SpectateButton->SetIsEnabled(true);
		SpectateButton->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SpectateButton->SetIsEnabled(false);
	}
}

bool UUW_PersonalResult::HasAliveTargets() const
{
	AT2PlayGameState* GS = GetWorld()->GetGameState<AT2PlayGameState>();
	if (!GS) return false;

	for (APlayerState* PS : GS->PlayerArray)
	{
		ASurvivorPlayerState* SurvivorPS = Cast<ASurvivorPlayerState>(PS);
		if (SurvivorPS && !SurvivorPS->bIsDead && !SurvivorPS->bIsEscaped)
		{
			return true;
		}
	}

	return false;
}
