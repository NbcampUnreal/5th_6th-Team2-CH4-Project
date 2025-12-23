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
#include "Gimmick/Player/ItemBase.h"
#include "GameFramework/Controller.h"
#include "Blueprint/UserWidget.h"
#include "UI/UW_RoundProgressBar.h"
#include "Team02.h"


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

void AT2PlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!bIsInteracting) return;

	CurrentInteractTime += DeltaTime;

	float Percent = CurrentInteractTime / RequiredInteractTime;

	if (AT2BaseController* PC = Cast<AT2BaseController>(GetController()))
	{
		PC->UpdateInteractUI(Percent);
	}
}

void AT2PlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AT2PlayerCharacter, bIsInteracting);
}

void AT2PlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	FirstPersonCamera->SetActive(true);
	ThirdPersonCamera->SetActive(false);
	GetMesh()->SetOwnerNoSee(true);
	FlashlightMesh->SetOwnerNoSee(true);

	if (IsLocallyControlled())
	{
		if (AT2BaseController* PC = Cast<AT2BaseController>(GetController()))
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

	GetWorldTimerManager().SetTimer(
		InteractTraceTimer,
		this,
		&AT2PlayerCharacter::UpdateInteractTarget,
		0.05f,
		true
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
		EIC->BindAction(InteractInput, ETriggerEvent::Started, this, &ThisClass::OnInteractStart);
		EIC->BindAction(InteractInput, ETriggerEvent::Completed, this, &ThisClass::OnInteractCompleted);
		EIC->BindAction(InteractInput, ETriggerEvent::Canceled, this, &ThisClass::OnInteractCanceled);
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
			AT2BaseController* PC = Cast<AT2BaseController>(GetOwner());
			{
				if (IsValid(PC) == true)
				{
					// GM->OnCharacterDead(PC);   When Player Died, Notice to GameMode 
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
		FirstPersonCamera->SetActive(true);
		ThirdPersonCamera->SetActive(false);

		GetMesh()->SetOwnerNoSee(true);
		FirstPersonArms->SetOwnerNoSee(false);
		FirstPersonArms->SetVisibility(false, true);

		FlashlightMesh->SetOwnerNoSee(true);

		bUseControllerRotationYaw = true;
		bUseControllerRotationPitch = true;

		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->RotationRate = FRotator(0, 500, 0);

		SpringArmComponent->bUsePawnControlRotation = false;
	}
	//ThirdPerson
	else
	{
		ThirdPersonCamera->SetActive(true);
		FirstPersonCamera->SetActive(false);

		GetMesh()->SetOwnerNoSee(false);
		FirstPersonArms->SetOwnerNoSee(true);
		FirstPersonArms->SetVisibility(false, true);

		FlashlightMesh->SetOwnerNoSee(false);

		bUseControllerRotationYaw = false;
		bUseControllerRotationPitch = false;

		GetCharacterMovement()->bOrientRotationToMovement = true;

		SpringArmComponent->bUsePawnControlRotation = true;
	}
}

void AT2PlayerCharacter::OnDeathMontageEneded()
{
	if (!HasAuthority()) return;

	DetachFromControllerPendingDestroy();
	Destroy();
}

void AT2PlayerCharacter::SetInteractableItem(AItemBase* Item)
{
	CurrentInteractItem = Item;
	//ShowInteractionUI(true);
}

void AT2PlayerCharacter::ClearInteractableItem(AItemBase* Item)
{
	if (CurrentInteractItem == Item)
	{
		CurrentInteractItem = nullptr;
		//ShowInteractionUI(false);
	}
}

void AT2PlayerCharacter::OnInteractStart()
{
	if (!IsLocallyControlled()) return;
	if (!CurrentInteractItem) return;
	if (bIsInteracting) return;

	bIsInteracting = true;
	CurrentInteractTime = 0.f;

	if (AT2BaseController* PC = Cast<AT2BaseController>(GetController()))
	{
		PC->StartInteractUI();
	}


	Server_BeginInteract(CurrentInteractItem);
	//StartInteractProgressUI();    상호작용체크/UI게이지 시작
}

void AT2PlayerCharacter::OnInteractCompleted()
{
	if (!IsLocallyControlled()) return;
	if (!bIsInteracting) return;

	bIsInteracting = false;

	if (AT2BaseController* PC = Cast<AT2BaseController>(GetController()))
	{
		PC->StopInteractUI();
	}

	Server_CompleteInteract(CurrentInteractItem);    //서버RPC호출 /실제 아이템 획득 요청
}

void AT2PlayerCharacter::OnInteractCanceled()
{
	if (!IsLocallyControlled()) return;
	if (!bIsInteracting) return;

	bIsInteracting = false;

	if (AT2BaseController* PC = Cast<AT2BaseController>(GetController()))
	{
		PC->StopInteractUI();
	}

	Server_CancelInteract(CurrentInteractItem);
	 // 타이머중지
}

void AT2PlayerCharacter::Server_BeginInteract_Implementation(AItemBase* Item)
{
	if (IsValid(Item) == false) return;


	Item->BeginInteract(this);
}

void AT2PlayerCharacter::Server_CompleteInteract_Implementation(AItemBase* Item)
{
	if (IsValid(Item) == false) return;

	Item->CompleteInteract(this);
}

void AT2PlayerCharacter::Server_CancelInteract_Implementation(AItemBase* Item)
{
	if (IsValid(Item) == false) return;

	Item->CanelInteract(this);
}


void AT2PlayerCharacter::AddNearbyItem(AItemBase* Item)
{
	if (!IsValid(Item)) return;

	NearbyItems.Add(Item);
}

void AT2PlayerCharacter::RemoveNearbyItem(AItemBase* Item)
{
	if (!IsValid(Item)) return;

	NearbyItems.Remove(Item);

	if (CurrentInteractItem == Item)
	{
		CurrentInteractItem = nullptr;
	}
}

void AT2PlayerCharacter::UpdateInteractTarget()
{
	FHitResult Hit;

	FVector Start = FirstPersonCamera->GetComponentLocation();
	FVector End = Start + FirstPersonCamera->GetForwardVector() * 300.f;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Interact,
		Params);

	//LineTrace TEST
//#if ENABLE_DRAW_DEBUG
//	DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 0.05f, 0, 1.f);
//#endif

	if (bHit)
	{
		if (AItemBase* HitItem = Cast<AItemBase>(Hit.GetActor()))
		{
			if (NearbyItems.Contains(HitItem))
			{
				CurrentInteractItem = HitItem;
				return;
			}
		}
	}
	if (bIsInteracting)
	{
		OnInteractCanceled();
	}

	if (AT2BaseController* PC = Cast<AT2BaseController>(GetController()))
	{
		PC->StopInteractUI();
	}

	CurrentInteractItem = nullptr;
	
}

