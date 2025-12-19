#include "Character/PlayerCharacter/T2PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "T2PlayGameMod.h"
#include "T2PlayGameState.h"
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

	if (!HasAuthority()) return;
	
	// 1. GameState에 알림 (SurvivorsAlive 감소)
	AT2PlayGameState* GS = GetWorld()->GetGameState<AT2PlayGameState>();
	if (GS)
	{
		GS->OnSurvivorDied();
		UE_LOG(LogTemp, Warning, TEXT("GameState->OnSurvivorDied called.  SurvivorsAlive:  %d"), GS->SurvivorsAlive);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameState is NULL in SetDead!"));
	}

	// 2. GameMode에 알림 (승패 체크)
	AT2PlayGameMod* GM = GetWorld()->GetAuthGameMode<AT2PlayGameMod>();
	if (GM)
	{
		GM->OnPlayerDied(nullptr);
		UE_LOG(LogTemp, Warning, TEXT("GameMode->OnPlayerDied called"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode is NULL in SetDead! "));
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
		// ī�޶� Ȱ��ȭ
		FirstPersonCamera->SetActive(true);
		ThirdPersonCamera->SetActive(false);

		// �þ߿��� �� �����
		GetMesh()->SetOwnerNoSee(true);
		FirstPersonArms->SetOwnerNoSee(false);
		FirstPersonArms->SetVisibility(true, true);

		FlashlightMesh->SetOwnerNoSee(true);

		// ��Ʈ�ѷ� ȸ�� �� ī�޶� ȸ��
		bUseControllerRotationYaw = true;
		bUseControllerRotationPitch = true;

		// �̵� ���� ���� ���� (ĳ���ʹ� ���콺 ȸ���� ���� ���� ȸ��)
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->RotationRate = FRotator(0, 500, 0);

		// spring arm ��Ȱ��ȭ
		SpringArmComponent->bUsePawnControlRotation = false;
	}
	//ThirdPerson
	else
	{
		// ī�޶� Ȱ��ȭ
		ThirdPersonCamera->SetActive(true);
		FirstPersonCamera->SetActive(false);

		// ĳ���� Ǯ�ٵ� �ٽ� ���̰�
		GetMesh()->SetOwnerNoSee(false);
		FirstPersonArms->SetOwnerNoSee(true);
		FirstPersonArms->SetVisibility(false, true);

		FlashlightMesh->SetOwnerNoSee(false);

		// ���콺 �������� �������� ȸ���� �����ϵ���
		bUseControllerRotationYaw = false;
		bUseControllerRotationPitch = false;

		// �̵� ������ �������� ĳ���� ȸ��
		GetCharacterMovement()->bOrientRotationToMovement = true;

		// ���������� ��Ʈ�ѷ� ȸ���� ���󰡰�
		SpringArmComponent->bUsePawnControlRotation = true;
	}
}

void AT2PlayerCharacter::OnDeathMontageEneded()
{
	if (!HasAuthority()) return;

	DetachFromControllerPendingDestroy();
	Destroy();
}
