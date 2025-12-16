#include "Controller/Player/T2PlayerController.h"

void AT2PlayerController::StartSpectate(AActor* Target)
{
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);

	ChangeState(NAME_Spectating);
	SetViewTargetWithBlend(Target, 0.5f);   //ViewTarget은 Controller권한 .. Character나 GM이 직접 건들면 구조 꼬임

	ShowSpectatorUI();
}

void AT2PlayerController::NextSpectate()
{
	//AActor* NextTarget = FindNextAlivePlayer();
	//if (IsValid(NextTarget) == true)
	//{
	//	SetViewTargetWithBlend(NextTarget, 0.3f);
	//}
}



void AT2PlayerController::ShowSpectatorUI()
{
	/*SpectatorWidget->AddToViewport();*/
}
