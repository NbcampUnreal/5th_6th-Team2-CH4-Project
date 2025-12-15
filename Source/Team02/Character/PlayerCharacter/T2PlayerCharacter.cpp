#include "Character/PlayerCharacter/T2PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Gimmick/Player/FlashlightComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"

AT2PlayerCharacter::AT2PlayerCharacter()
{
	FlashlightMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FlashlightMesh"));
	FlashlightMesh->SetupAttachment(GetMesh());

	FlashlightComp = CreateDefaultSubobject<UFlashlightComponent>(TEXT("FlashlightComponent"));

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 150.f;
	GetCharacterMovement()->SetUpdatedComponent(GetCapsuleComponent());

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 300.f;
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->bEnableCameraLag = true;
	SpringArmComponent->CameraLagSpeed = 10.f;

	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCamera->SetupAttachment(SpringArmComponent);
	ThirdPersonCamera->bUsePawnControlRotation = false;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), TEXT("head"));
	FirstPersonCamera->bUsePawnControlRotation = true;

	FirstPersonArms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonArmMesh"));
	FirstPersonArms->SetupAttachment(FirstPersonCamera);
	FirstPersonArms->SetOnlyOwnerSee(true);
	FirstPersonArms->SetCastShadow(false);
	FirstPersonArms->SetVisibility(false, true);

	bUseControllerRotationYaw = false;

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

void AT2PlayerCharacter::Tick(float DeltaTime)
{
	UCapsuleComponent* Cap = GetCapsuleComponent();
	UCharacterMovementComponent* Move = GetCharacterMovement();

	const float StandHH = Cap->GetUnscaledCapsuleHalfHeight();
	const float CrouchHH = Move->CrouchedHalfHeight;
	const float Radius = Cap->GetUnscaledCapsuleRadius();

	const FVector BaseLoc = Cap->GetComponentLocation();

	// 현재 캡슐 (빨강)
	DrawDebugCapsule(
		GetWorld(),
		BaseLoc,
		StandHH,
		Radius,
		Cap->GetComponentQuat(),
		FColor::Red,
		false, 0.f, 0, 2.f
	);

	// 바닥 기준으로 보정된 crouch 캡슐 (초록)
	const float HalfHeightAdjust = StandHH - CrouchHH;
	const FVector CrouchCenter = BaseLoc - FVector(0, 0, HalfHeightAdjust);

	DrawDebugCapsule(
		GetWorld(),
		CrouchCenter,
		CrouchHH,
		Radius,
		Cap->GetComponentQuat(),
		FColor::Green,
		false, 0.f, 0, 2.f
	);
}

void AT2PlayerCharacter::HandleCrouchInput(const FInputActionValue& InValue)
{
	if(IsLocallyControlled()==false)
	{
		return;
	}

	BTC = !BTC;
	Server_ToggleCrouch();
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

void AT2PlayerCharacter::ToggleCrouch()
{
	if (GetCharacterMovement()->IsCrouching())
	{
		UnCrouch();

		UE_LOG(LogTemp, Error, TEXT("UnCrouch"));
		UE_LOG(LogTemp, Error, TEXT("%d"), GetCharacterMovement()->IsCrouching());
	}
	else
	{
		Crouch();
		UE_LOG(LogTemp, Error, TEXT("Crouch"));
		UE_LOG(LogTemp, Error, TEXT("%d"), GetCharacterMovement()->IsCrouching());
	}

}

void AT2PlayerCharacter::Server_ToggleCrouch_Implementation()
{
	//ToggleCrouch();

	//UE_LOG(LogTemp, Error,
	//	TEXT("SERVER RPC Role=%d CanCrouch=%d Falling=%d"),
	//	(int32)GetLocalRole(),
	//	GetCharacterMovement()->CanCrouchInCurrentState(),
	//	GetCharacterMovement()->IsFalling());

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}

	UE_LOG(LogTemp, Warning,
		TEXT("Wants=%d , IsCrouched=%d , CanCrouch=%d , Falling=%d"),
		GetCharacterMovement()->bWantsToCrouch,
		bIsCrouched,
		GetCharacterMovement()->CanCrouchInCurrentState(),
		GetCharacterMovement()->IsFalling()
	);
}

void AT2PlayerCharacter::HandleViewModeInput(const FInputActionValue& InValue)
{
	bIsFirstPerson = !bIsFirstPerson;

	//FirstPerson
	if (bIsFirstPerson)
	{
		// 카메라 활성화
		FirstPersonCamera->SetActive(true);
		ThirdPersonCamera->SetActive(false);

		// 시야에서 몸 숨기기
		GetMesh()->SetOwnerNoSee(true);
		FirstPersonArms->SetOwnerNoSee(false);
		FirstPersonArms->SetVisibility(true, true);

		// 컨트롤러 회전 → 카메라 회전
		bUseControllerRotationYaw = true;
		bUseControllerRotationPitch = true;

		// 이동 방향 관련 세팅 (캐릭터는 마우스 회전에 따라 같이 회전)
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->RotationRate = FRotator(0, 500, 0);

		// spring arm 비활성화
		SpringArmComponent->bUsePawnControlRotation = false;
	}
	//ThirdPerson
	else
	{
		// 카메라 활성화
		ThirdPersonCamera->SetActive(true);
		FirstPersonCamera->SetActive(false);

		// 캐릭터 풀바디 다시 보이게
		GetMesh()->SetOwnerNoSee(false);
		FirstPersonArms->SetOwnerNoSee(true);
		FirstPersonArms->SetVisibility(false, true);

		// 마우스 움직임은 스프링암 회전만 제어하도록
		bUseControllerRotationYaw = false;
		bUseControllerRotationPitch = false;

		// 이동 방향을 기준으로 캐릭터 회전
		GetCharacterMovement()->bOrientRotationToMovement = true;

		// 스프링암이 컨트롤러 회전을 따라가게
		SpringArmComponent->bUsePawnControlRotation = true;
	}
}
