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
		AT2GameStateBase* GS = GetWorld()->GetGameState<AT2GameStateBase>();
		if (IsValid(GS))
		{
			int32 OldCount = GS->GetKeyCount();
			GS->AddKeyCount(1);

			UE_LOG(LogTemp, Warning, TEXT(" Key collected! Count: %d → %d"), OldCount, GS->GetKeyCount());
	
			
			AT2GameModeBase* GM = Cast<AT2GameModeBase>(GetWorld()->GetAuthGameMode());
			if (IsValid(GM))
			{
				GM->OnKeyCollected(GS->GetKeyCount());
				UE_LOG(LogTemp, Warning, TEXT(" GameMode::OnKeyCollected(%d) called"), 
					GS->GetKeyCount());
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT(" GameMode not found!"));
			}
		}

		Destroy();
	}
}

void AExitKey::BeginInteract(class AT2PlayerCharacter* Player)
{
	Super::BeginInteract(Player);
}
