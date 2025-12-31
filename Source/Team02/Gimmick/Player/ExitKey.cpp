#include "Gimmick/Player/ExitKey.h"

#include "T2PlayGameState.h"
#include "Components/CapsuleComponent.h"
#include "Character/PlayerCharacter/T2PlayerCharacter.h"
#include "Controller/T2BaseController.h"
#include "GameState/T2GameStateBase.h" 
#include "GameMode/T2GameModeBase.h"

// Sets default values
AExitKey::AExitKey()
{
	bReplicates = true;
 	PrimaryActorTick.bCanEverTick = false;

}


void AExitKey::BeginPlay()
{
	Super::BeginPlay();

}

void AExitKey::CompleteInteract(AT2PlayerCharacter* Player)
{
	if (!HasAuthority()) return;

	AT2BaseController* PlayerController = Cast<AT2BaseController>(Player->GetController());
	if (InteractingPC != PlayerController) return;
	
	if (IsValid(Player))
	{
		AT2PlayGameState* GS = GetWorld()->GetGameState<AT2PlayGameState>();
		if (IsValid(GS))
		{
			int32 OldCount = GS->TotalKeyCount; 
			GS->AddKeyCount(1);
		}

		Destroy();
	}
}

void AExitKey::BeginInteract(class AT2PlayerCharacter* Player)
{
	Super::BeginInteract(Player);
}
