// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Killer/Notify/KillerAttackHitBox.h"

#include "Character/KillerCharacter/T2KillerCharacter.h" 
#include "PlayerState/Player/SurvivorPlayerState.h" 
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"

void UKillerAttackHitBox::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                      float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		HitActors.Empty(); 
		PreviousWeaponLocation = MeshComp->GetSocketLocation(TEXT("AxeSocket"));
	}
}

void UKillerAttackHitBox::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	
	if (!MeshComp || !MeshComp->GetOwner() || !MeshComp->GetOwner()->HasAuthority()) 
	{
		return;
	}

	AT2KillerCharacter* KillerCharacter = Cast<AT2KillerCharacter>(MeshComp->GetOwner());
	if (!KillerCharacter)
	{
		return;
	}
	
	FVector KillerLocation = KillerCharacter->GetActorLocation();
	FVector ForwardVector = KillerCharacter->GetActorForwardVector();
	
	FVector BoxHalfSize(80.0f, 70.0f, 50.0f); 
	float TraceDistance = 100.0f; 
	
	FVector StartLocation = KillerLocation + FVector(0, 0, 50.0f); 
	FVector EndLocation = StartLocation + (ForwardVector * TraceDistance);
	
	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(KillerCharacter);
	
	for (AActor* HitActor : HitActors)
	{
		Params.AddIgnoredActor(HitActor);
	}
	
	FQuat BoxRotation = KillerCharacter->GetActorRotation().Quaternion();
	
	bool bHit = MeshComp->GetWorld()->SweepMultiByChannel(
		HitResults,
		StartLocation,
		EndLocation,
		BoxRotation,
		ECC_Pawn,
		FCollisionShape::MakeBox(BoxHalfSize),
		Params
	);

	if (bHit)
	{
		for (const FHitResult& HitResult : HitResults)
		{
			APawn* HitPawn = Cast<APawn>(HitResult.GetActor());
			if (HitPawn && HitPawn != KillerCharacter)
			{
				if (HitActors.Contains(HitPawn))
				{
					continue;
				}

				ASurvivorPlayerState* SurvivorPS = Cast<ASurvivorPlayerState>(HitPawn->GetPlayerState());
				
				if (!SurvivorPS)
				{
					if (Cast<AT2KillerCharacter>(HitPawn))
					{
						continue;
					}
					continue;
				}
				
				SurvivorPS->ApplyDamage(40.f);
				KillerCharacter->OnHitSuccessful(HitPawn, HitResult.ImpactPoint);
				HitActors.AddUnique(HitPawn);
			}
		}
	}

	PreviousWeaponLocation = MeshComp->GetSocketLocation(TEXT("AxeSocket"));
}

void UKillerAttackHitBox::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	HitActors.Empty();
}