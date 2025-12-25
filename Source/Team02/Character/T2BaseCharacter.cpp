// Fill out your copyright notice in the Description page of Project Settings.

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

    // 클라이언트용: BeginPlay에서 설정
    if (IsLocallyControlled())
    {
        SetupBaseEnhancedInput();
    }
}

void AT2BaseCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    // 서버용: PossessedBy에서 설정
    APlayerController* PC = Cast<APlayerController>(NewController);
    if (PC && PC->IsLocalPlayerController())
    {
        SetupBaseEnhancedInput();
        UE_LOG(LogTemp, Warning, TEXT("T2BaseCharacter::PossessedBy - Input setup for Server Local Player"));
    }
}

void AT2BaseCharacter::SetupBaseEnhancedInput()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    if (UEnhancedInputLocalPlayerSubsystem* EILPS =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
    {
        if (InputMappingContext)
        {
            EILPS->AddMappingContext(InputMappingContext, 0);
            UE_LOG(LogTemp, Warning, TEXT("T2BaseCharacter:  InputMappingContext added! "));
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