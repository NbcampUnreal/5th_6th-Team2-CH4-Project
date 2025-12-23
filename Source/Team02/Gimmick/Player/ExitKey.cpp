#include "Gimmick/Player/ExitKey.h"
#include "Components/CapsuleComponent.h"
#include "Character/PlayerCharacter/T2PlayerCharacter.h"
#include "T2PlayGameState.h"

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

	if (IsValid(Player) == true)
	{
		AT2PlayGameState* GS = GetWorld()->GetGameState<AT2PlayGameState>();
		if (IsValid(GS))
		{
			GS->AddKeyCount(1);
		}

		Destroy();
	}
}
