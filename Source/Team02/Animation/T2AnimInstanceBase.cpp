// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/T2AnimInstanceBase.h"

#include "Character/T2BaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UT2AnimInstanceBase::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter = Cast<AT2BaseCharacter>(GetOwningActor());
	if (IsValid(OwnerCharacter))
	{
		OwnerCharacterMovementComponent = OwnerCharacter->GetCharacterMovement();
	}
}

void UT2AnimInstanceBase::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!IsValid(OwnerCharacter) || !IsValid(OwnerCharacterMovementComponent))
	{
		return;
	}

	Velocity = OwnerCharacterMovementComponent->Velocity;
	GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.f).Size();
	
	bShouldMove = (!OwnerCharacterMovementComponent->GetCurrentAcceleration().IsNearlyZero()) && (GroundSpeed > 3.f);
	
	bIsFalling = OwnerCharacterMovementComponent->IsFalling();
}
