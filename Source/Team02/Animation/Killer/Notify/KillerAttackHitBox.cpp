// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Killer/Notify/KillerAttackHitBox.h"

#include "Character/KillerCharacter/T2KillerCharacter.h" 
#include "PlayerState/Player/SurvivorPlayerState.h" 
#include "DrawDebugHelpers.h"

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
	
	FVector CurrentWeaponLocation = MeshComp->GetSocketLocation(TEXT("AxeSocket"));
	
	FVector StartLocation = PreviousWeaponLocation;
	FVector EndLocation = CurrentWeaponLocation;
	
	const float CapsuleRadius = 40.0f; 
	const float CapsuleHalfHeight = 150.0f;
	FCollisionShape TraceShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);
	
	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(KillerCharacter);
	
	for (AActor* HitActor : HitActors)
	{
		Params.AddIgnoredActor(HitActor);
	}
	
	bool bHit = MeshComp->GetWorld()->SweepMultiByChannel(
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		ECC_Pawn, 
		TraceShape, 
		Params
	);

	if (bHit)
	{
		for (const FHitResult& HitResult : HitResults)
		{
			APawn* HitPawn = Cast<APawn>(HitResult.GetActor());
			if (HitPawn)
			{
				UE_LOG(LogTemp, Warning, TEXT("SERVER: Sweep Hit Pawn: %s"), *HitPawn->GetName());
    
				if (HitPawn == KillerCharacter) continue;

				APlayerState* GenericPS = HitPawn->GetPlayerState();
    
				if (GenericPS)
				{
					UE_LOG(LogTemp, Warning, TEXT("SERVER: Hit Pawn PlayerState Found. Class Name: %s"), *GenericPS->GetClass()->GetName());
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("SERVER: Hit Pawn %s has NO PlayerState."), *HitPawn->GetName());
				}

				ASurvivorPlayerState* SurvivorPS = Cast<ASurvivorPlayerState>(HitPawn->GetPlayerState());
    
				if (!SurvivorPS)
				{
					if (Cast<AT2KillerCharacter>(HitPawn))
					{
						UE_LOG(LogTemp, Warning, TEXT("SERVER: Hit Pawn is the Killer Character itself. Ignoring."));
						continue; 
					}
        
					UE_LOG(LogTemp, Error, TEXT("SERVER: Hit Pawn %s does NOT have a valid SurvivorPlayerState."), *HitPawn->GetName());
					continue;
				}
    
				UE_LOG(LogTemp, Warning, TEXT("SERVER HIT SUCCESS: Target %s, Applying Damage."), *HitPawn->GetName());
    
				SurvivorPS->ApplyDamage(40.f);

				KillerCharacter->OnHitSuccessful(HitPawn, HitResult.ImpactPoint);

				HitActors.AddUnique(HitResult.GetActor());

				
			}
		}

		
	}

	PreviousWeaponLocation = CurrentWeaponLocation;
	
	DrawDebugCapsule(
		MeshComp->GetWorld(),
		(StartLocation + EndLocation) / 2.f, 
		CapsuleHalfHeight,
		CapsuleRadius,
		FRotationMatrix::MakeFromX(EndLocation - StartLocation).ToQuat(),
		bHit ? FColor::Red : FColor::Green,
		false,
		FrameDeltaTime * 2.0f, // <-- 짧은 시간 동안만 표시
		0,
		3.f
		);
}

void UKillerAttackHitBox::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
}
