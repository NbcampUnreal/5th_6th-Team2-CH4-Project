// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/T2BaseCharacter.h"

#include "Character/T2BaseCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PlayerState/Player/SurvivorPlayerState.h"


AT2BaseCharacter::AT2BaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
}

void AT2BaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocallyControlled())
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* EILPS = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			{
				EILPS->AddMappingContext(InputMappingContext, 0);
			}
		}
	}
}

void AT2BaseCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::HandleMoveInput);
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::HandleLookInput);
	}
}

void AT2BaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AT2BaseCharacter::HandleMoveInput(const FInputActionValue& InValue)
{
	if (!Controller) return;

	const FVector2D InMovementVector = InValue.Get<FVector2D>();
	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator ControlYawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(ControlYawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(ControlYawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, InMovementVector.X);
	AddMovementInput(RightDirection, InMovementVector.Y);
}

void AT2BaseCharacter::HandleLookInput(const FInputActionValue& InValue)
{
	if (!Controller) return;
	
	const FVector2D InLookVector = InValue.Get<FVector2D>();
	AddControllerYawInput(InLookVector.X);
	AddControllerPitchInput(InLookVector.Y);
}

void AT2BaseCharacter::AnimNotify_DeathEnd()
{
	UE_LOG(LogTemp, Warning, TEXT("AnimNotify_DeathEnd | Authority=%d"), HasAuthority());

	if (!HasAuthority())
	{
		Server_NotifyDeathMontageEnded();
	}
}

void AT2BaseCharacter::Multicast_PlayHitMontage_Implementation()
{
	if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
	{
		if (HitMontage)
		{
			Anim->Montage_Play(HitMontage);
		}
	}
}

void AT2BaseCharacter::Multicast_PlayDeathMontage_Implementation()
{
	if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
	{
		if (DeathMontage)
		{
			Anim->Montage_Play(DeathMontage);
		}
	}
}

void AT2BaseCharacter::Server_NotifyDeathMontageEnded_Implementation()
{
	if (bDeathMontageEndedHandled) return;
	bDeathMontageEndedHandled = true;

	DetachFromControllerPendingDestroy();
	Destroy();
}



