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
	
	// 킬러 앞쪽으로 박스 트레이스
	FVector KillerLocation = KillerCharacter->GetActorLocation();
	FVector ForwardVector = KillerCharacter->GetActorForwardVector();
	
	// 박스 설정 (폭, 높이, 깊이)
	FVector BoxHalfSize(80.0f, 70.0f, 50.0f); // X=앞뒤, Y=좌우, Z=상하
	float TraceDistance = 100.0f; // 앞으로 얼마나 멀리 체크할지
	
	FVector StartLocation = KillerLocation + FVector(0, 0, 50.0f); // 약간 위에서 시작
	FVector EndLocation = StartLocation + (ForwardVector * TraceDistance);
	
	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(KillerCharacter);
	
	for (AActor* HitActor : HitActors)
	{
		Params.AddIgnoredActor(HitActor);
	}
	
	// 박스 트레이스 실행
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
				// 이미 맞은 액터 스킵
				if (HitActors.Contains(HitPawn))
				{
					continue;
				}

				UE_LOG(LogTemp, Warning, TEXT("SERVER: Box Sweep Hit Pawn: %s"), *HitPawn->GetName());

				ASurvivorPlayerState* SurvivorPS = Cast<ASurvivorPlayerState>(HitPawn->GetPlayerState());
				
				if (!SurvivorPS)
				{
					if (Cast<AT2KillerCharacter>(HitPawn))
					{
						continue;
					}
					
					UE_LOG(LogTemp, Error, TEXT("SERVER: Hit Pawn %s has NO SurvivorPlayerState."), *HitPawn->GetName());
					continue;
				}
				
				UE_LOG(LogTemp, Warning, TEXT("SERVER HIT SUCCESS: Target %s, Applying Damage."), *HitPawn->GetName());
				
				SurvivorPS->ApplyDamage(40.f);
				KillerCharacter->OnHitSuccessful(HitPawn, HitResult.ImpactPoint);
				HitActors.AddUnique(HitPawn);
			}
		}
	}

	// 디버그 박스 그리기
	FVector BoxCenter = (StartLocation + EndLocation) / 2.0f;
	DrawDebugBox(
		MeshComp->GetWorld(),
		BoxCenter,
		BoxHalfSize,
		BoxRotation,
		bHit ? FColor::Red : FColor::Green,
		false,
		FrameDeltaTime * 2.0f,
		0,
		3.0f
	);

	PreviousWeaponLocation = MeshComp->GetSocketLocation(TEXT("AxeSocket"));
}

void UKillerAttackHitBox::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	HitActors.Empty();
}