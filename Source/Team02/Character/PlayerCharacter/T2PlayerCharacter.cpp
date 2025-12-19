#include "Character/PlayerCharacter/T2PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Gimmick/Player/FlashlightComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "PlayerState/Player/SurvivorPlayerState.h"
#include "Components/SpotLightComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameMode/T2GameModeBase.h"
#include "GameMode/TestGameMode.h"


AT2PlayerCharacter::AT2PlayerCharacter()
{
	bReplicates = true;
	GetCharacterMovement()->PrimaryComponentTick.bCanEverTick = true;
	GetCharacterMovement()->PrimaryComponentTick.bStartWithTickEnabled = true;

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

	FP_ArmPivot = CreateDefaultSubobject<USceneComponent>(TEXT("FP_ArmPivot"));
	FP_ArmPivot->SetupAttachment(FirstPersonCamera);

	FP_ArmPivot->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	FP_ArmPivot->SetRelativeLocation(FVector(0.f, 0.f, -10.f));

	FirstPersonArms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonArmMesh"));
	FirstPersonArms->SetupAttachment(FP_ArmPivot);
	FirstPersonArms->SetOnlyOwnerSee(true);
	FirstPersonArms->SetCastShadow(false);
	FirstPersonArms->SetVisibility(false, true);

	bUseControllerRotationYaw = false;

	Flashlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLightComponent"));
	Flashlight->SetupAttachment(FirstPersonCamera);
	Flashlight->SetVisibility(false);

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

	PlayerInputComponent->BindKey(
		FKey(EKeys::F),
		IE_Pressed,
		this,
		&ThisClass::HandleFlashlightInput
	);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(CrouchInput, ETriggerEvent::Started, this, &ThisClass::HandleCrouchInput);
		EIC->BindAction(ViewModeInput, ETriggerEvent::Started, this, &ThisClass::HandleViewModeInput);
	}

}

void AT2PlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	//ASurvivorPlayerState* PS = GetPlayerState<ASurvivorPlayerState>();
	//if (PS)
	//{
	//	PS->OnHPChanged.AddUObject(this, &ThisClass::HandleHPChanged);
	//	UE_LOG(LogTemp, Warning, TEXT("PlayerState arrived, delegate bound"));
	//}
}

void AT2PlayerCharacter::HandleCrouchInput(const FInputActionValue& InValue)
{
	if(IsLocallyControlled()==false)
	{
		return;
	}
	
	bool bTargetrState = !bIsCrouched;
	if (bTargetrState)
	{
		Crouch();
	}
	else
	{
		UnCrouch();
	}
	
	Server_ToggleCrouch();
}

void AT2PlayerCharacter::HandleFlashlightInput()
{
	if (FlashlightComp)
	{
		FlashlightComp->Server_ToggleFlashlight();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FlashlightComp is Invalid"));
	}
}

void AT2PlayerCharacter::HandleHPChanged(float CurrentHP, float MaxHP)
{
	/*if (CurrentHP > 0)
	{
		Multicast_PlayHitMontage();
	}*/
}

float AT2PlayerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority())
	{
		return 0.f;
	}

	ASurvivorPlayerState* PS = GetPlayerState<ASurvivorPlayerState>();
	if (IsValid(PS) == true)
	{
		PS->ApplyDamage(DamageAmount);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerState invalid in TakeDamage"));
		return 0.f;
	}

	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	return DamageAmount;
}

void AT2PlayerCharacter::OnDeath()
{
	if (HasAuthority())
	{
		Multicast_PlayDeathMontage();

		ATestGameMode* GM = GetWorld()->GetAuthGameMode<ATestGameMode>();
		if (IsValid(GM) == true)
		{
			AT2PlayerController* PC = Cast<AT2PlayerController>(GetOwner());
			{
				if (IsValid(PC) == true)
				{
					GM->OnCharacterDead(PC);
				}
			}
		}
	}
}

void AT2PlayerCharacter::Server_ToggleCrouch_Implementation()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
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

		FlashlightMesh->SetOwnerNoSee(true);

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

		FlashlightMesh->SetOwnerNoSee(false);

		// 마우스 움직임은 스프링암 회전만 제어하도록
		bUseControllerRotationYaw = false;
		bUseControllerRotationPitch = false;

		// 이동 방향을 기준으로 캐릭터 회전
		GetCharacterMovement()->bOrientRotationToMovement = true;

		// 스프링암이 컨트롤러 회전을 따라가게
		SpringArmComponent->bUsePawnControlRotation = true;
	}
}

void AT2PlayerCharacter::OnDeathMontageEneded()
{
	if (!HasAuthority()) return;

	DetachFromControllerPendingDestroy();
	Destroy();
}
