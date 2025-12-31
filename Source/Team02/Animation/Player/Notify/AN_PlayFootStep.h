#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_PlayFootStep.generated.h"

UCLASS()
class TEAM02_API UAN_PlayFootStep : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
	FName FootSocketName = TEXT("foot_l");

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

};
