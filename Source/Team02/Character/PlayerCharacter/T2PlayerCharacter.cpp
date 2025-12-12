#include "Character/PlayerCharacter/T2PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Gimmick/Player/FlashlightComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

AT2PlayerCharacter::AT2PlayerCharacter()
{
	FlashlightMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FlashlightMesh"));
	FlashlightMesh->SetupAttachment(GetMesh());

	FlashlightComp = CreateDefaultSubobject<UFlashlightComponent>(TEXT("FlashlightComponent"));

	GetCharacterMovement()->MaxWalkSpeed = 300.f;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);

	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCamera->SetupAttachment(SpringArmComponent);
	ThirdPersonCamera->Activate(false);

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), TEXT("head"));
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->Activate(true);

	FirstPersonArms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonArmMesh"));
	FirstPersonArms->SetupAttachment(RootComponent);
}

void AT2PlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocallyControlled())
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* EILPS = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			{
				EILPS->AddMappingContext(PlayerInputMappingContext, 1);
			}
		}
	}

	FlashlightMesh->AttachToComponent(
		GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		FName("hand_rSocket")
	);
}

void AT2PlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(CrouchInput, ETriggerEvent::Started, this, &ThisClass::HandleCrouchInput);
		EIC->BindAction(FlashlightInput, ETriggerEvent::Started, this, &ThisClass::HandleFlashlightInput);
		EIC->BindAction(ViewModeInput, ETriggerEvent::Started, this, &ThisClass::HandleViewModeInput);
	}
}

void AT2PlayerCharacter::HandleCrouchInput(const FInputActionValue& InValue)
{
	bIsCrouching = !bIsCrouching;

	if (bIsCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = 150.f;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = 300.f;
	}
}

void AT2PlayerCharacter::HandleFlashlightInput(const FInputActionValue& InValue)
{
	if (FlashlightComp)
	{
		FlashlightComp->ToggleFlashlight();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FlashlightComp is Invalid"));
	}
}

void AT2PlayerCharacter::HandleViewModeInput(const FInputActionValue& InValue)
{
	bShowFullBody = !bShowFullBody;

	//ThirdPerson
	if (bShowFullBody)
	{
		GetMesh()->SetVisibility(true, true);
		FirstPersonArms->SetVisibility(false, true);

		FirstPersonCamera->Activate(false);
		ThirdPersonCamera->Activate(true);

		bUseControllerRotationPitch = false;
		bUseControllerRotationYaw = true;
		bUseControllerRotationRoll = false;

		GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);

		SpringArmComponent->TargetArmLength = 300.f;

		ThirdPersonCamera->bUsePawnControlRotation = false;
		FirstPersonCamera->bUsePawnControlRotation = false;
	}
	//FirstPerson
	else
	{
		GetMesh()->SetVisibility(false, true);
		FirstPersonArms->SetVisibility(true, true);

		FirstPersonCamera->Activate(true);
		ThirdPersonCamera->Activate(false);

		FirstPersonCamera->bUsePawnControlRotation = true;
		ThirdPersonCamera->bUsePawnControlRotation = false;
	}
}
