#include "Animation/Player/Notify/AN_DeathMontageEnded.h"
#include "Character/PlayerCharacter/T2PlayerCharacter.h"

void UAN_DeathMontageEnded::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp)) return;

	APawn* Pawn = Cast<APawn>(MeshComp->GetOwner());
	if (!Pawn) return;

	if (Pawn->HasAuthority())
	{
		if (AT2PlayerCharacter* Survivor = Cast<AT2PlayerCharacter>(Pawn))
		{
			Survivor->OnDeathMontageEneded();
		}
	}


}
