#include "Animation/Player/Notify/AN_PlayFootStep.h"
#include "Character/PlayerCharacter/T2PlayerCharacter.h"

void UAN_PlayFootStep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp)) return;

	APawn* Pawn = Cast<APawn>(MeshComp->GetOwner());
	if (!Pawn) return;

	if (AT2PlayerCharacter* Survivor = Cast<AT2PlayerCharacter>(Pawn))
	{
		Survivor->HandleFootstep(FootSocketName);
	}

}
