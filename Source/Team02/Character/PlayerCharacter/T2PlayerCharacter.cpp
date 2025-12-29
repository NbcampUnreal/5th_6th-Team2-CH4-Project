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
#include "PlayerState/Player/InventoryComponent.h"
#include "Components/SpotLightComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameMode/T2GameModeBase.h"
#include "Gimmick/Player/ItemBase.h"
#include "GameFramework/Controller.h"
#include "Blueprint/UserWidget.h"
#include "UI/UW_RoundProgressBar.h"
#include "Controller/T2BaseController.h"
#include "Team02.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"


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
		EIC->BindAction(UseSlot1, ETriggerEvent::Started, this, &ThisClass::HandleUseSlot1);
		EIC->BindAction(UseSlot2, ETriggerEvent::Started, this, &ThisClass::HandleUseSlot2);
		EIC->BindAction(UseSlot3, ETriggerEvent::Started, this, &ThisClass::HandleUseSlot3);
		EIC->BindAction(UseSlot4, ETriggerEvent::Started, this, &ThisClass::HandleUseSlot4);
		EIC->BindAction(UseSlot5, ETriggerEvent::Started, this, &ThisClass::HandleUseSlot5);
	}
}

void AT2PlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
}

void AT2PlayerCharacter::HandleCrouchInput(const FInputActionValue& InValue)
{
	if (IsLocallyControlled() == false)
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
	// 이미 죽은 상태면 무시 (중복 호출 방지)
	if (bIsDead) return;
	bIsDead = true;

	UE_LOG(LogTemp, Warning, TEXT("=== OnDeath Called for %s ==="), *GetName());

	if (!HasAuthority()) return;

	// 1. 사망 몽타주 재생
	Multicast_PlayDeathMontage();

	// 2. 입력 비활성화
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}

	// 3. 충돌 비활성화 (킬러가 더 이상 때릴 수 없게)
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 4. 결과창 표시 (사망)
	if (AT2BaseController* T2PC = Cast<AT2BaseController>(GetController()))
	{
		T2PC->Client_ShowPersonalResult(false);  // false = 사망
	}

	// 5. GameMode에 알림
	AT2PlayGameMod* GM = GetWorld()->GetAuthGameMode<AT2PlayGameMod>();
	if (GM)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		GM->OnCharacterDead(PC);
		UE_LOG(LogTemp, Warning, TEXT("GameMode->OnCharacterDead called"));
	}

	// 6. 일정 시간 후 시체 제거 (3초)
	FTimerHandle DeathTimerHandle;
	GetWorldTimerManager().SetTimer(DeathTimerHandle, this, &AT2PlayerCharacter::DestroyAfterDeath, 3.0f, false);
}

void AT2PlayerCharacter::DestroyAfterDeath()
{
	if (!HasAuthority()) return;

	UE_LOG(LogTemp, Warning, TEXT("DestroyAfterDeath:  Destroying %s"), *GetName());

	DetachFromControllerPendingDestroy();
	Destroy();
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
	// 이제 사용하지 않음 - DestroyAfterDeath()로 대체
}

void AT2PlayerCharacter::SetInteractableItem(AItemBase* Item)
{
	CurrentInteractItem = Item;
}

void AT2PlayerCharacter::ClearInteractableItem(AItemBase* Item)
{
	if (CurrentInteractItem == Item)
	{
		CurrentInteractItem = nullptr;
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

	UGameplayStatics::PlaySound2D(this, ItemAcquisition);

	Server_CompleteInteract(CurrentInteractItem);
}

void AT2PlayerCharacter::OnInteractCanceled()
{
	if (!IsLocallyControlled()) return;
	if (!bIsInteracting) return;

	bIsInteracting = false;
	CurrentInteractTime = 0.f;

	if (AT2BaseController* PC = Cast<AT2BaseController>(GetController()))
	{
		PC->StopInteractUI();
	}

	Server_CancelInteract(CurrentInteractItem);
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

	if (bHit)
	{
		if (AItemBase* HitItem = Cast<AItemBase>(Hit.GetActor()))
		{
			if (NearbyItems.Contains(HitItem))
			{
				if (HighlightedItem && HighlightedItem != HitItem)
				{
					HighlightedItem->SetOutlineEnabled(false);
				}

				HitItem->SetOutlineEnabled(true);

				HighlightedItem = HitItem;
				CurrentInteractItem = HitItem;
				return;
			}
		}
	}

	if (HighlightedItem)
	{
		HighlightedItem->SetOutlineEnabled(false);
		HighlightedItem = nullptr;
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

void AT2PlayerCharacter::HandleUseSlot1(const FInputActionValue& InValue)
{
	Server_UseInventorySlot(0);
}

void AT2PlayerCharacter::HandleUseSlot2(const FInputActionValue& InValue)
{
	Server_UseInventorySlot(1);
}

void AT2PlayerCharacter::HandleUseSlot3(const FInputActionValue& InValue)
{
	Server_UseInventorySlot(2);
}

void AT2PlayerCharacter::HandleUseSlot4(const FInputActionValue& InValue)
{
	Server_UseInventorySlot(3);
}

void AT2PlayerCharacter::HandleUseSlot5(const FInputActionValue& InValue)
{
	Server_UseInventorySlot(4);
}

void AT2PlayerCharacter::Server_UseInventorySlot_Implementation(int32 SlotIndex)
{
	ASurvivorPlayerState* PS = GetPlayerState<ASurvivorPlayerState>();
	if (!PS || !PS->InventoryComponent) return;

	PS->InventoryComponent->UseSlot(SlotIndex);
}

void AT2PlayerCharacter::HandleFootstep(FName SocketName)
{
	if (!IsLocallyControlled()) return;

	if (!HasAuthority())
	{
		Server_HandleFootstep(SocketName);
		return;
	}
}

void AT2PlayerCharacter::Server_HandleFootstep_Implementation(FName SocketName)
{
	if (!GetMesh()) return;

	const FVector FootLocation = GetMesh()->GetSocketLocation(SocketName);

	Multicast_PlayFootstep(FootLocation);
}


void AT2PlayerCharacter::Multicast_PlayFootstep_Implementation(const FVector& Location)
{
	if (!FootstepSound) return;

	UE_LOG(LogTemp, Warning,
		TEXT("FOOTSTEP MULTICAST | Pawn=%s | Authority=%d | LocalControlled=%d | NetMode=%d"),
		*GetNameSafe(this),
		HasAuthority(),
		IsLocallyControlled(),
		(int32)GetNetMode());

	UGameplayStatics::PlaySoundAtLocation(
		this,
		FootstepSound,
		Location,
		1.0f,
		1.0f,
		0.0f,
		FootstepAttenuation
	);
}

void AT2PlayerCharacter::Client_UsePotionEffect_Implementation()
{
	UGameplayStatics::PlaySound2D(this, UsePotion);
}

void AT2PlayerCharacter::Client_ChangeBatteryEffect_Implementation()
{
	UGameplayStatics::PlaySound2D(this, ChangeBattery);
}
