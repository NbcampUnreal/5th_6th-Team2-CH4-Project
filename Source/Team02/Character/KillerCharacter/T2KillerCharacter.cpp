// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/KillerCharacter/T2KillerCharacter.h"

#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Gimmick/KillerLandTrap.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Component/T2CooldownComponent.h"
#include "Component/T2FogComponent.h"
#include "Component/T2KillerDetectionComponent.h"  
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerState/Player/SurvivorPlayerState.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Components/SpotLightComponent.h"

AT2KillerCharacter::AT2KillerCharacter()
{
	GetCharacterMovement()->MaxWalkSpeed = 250.f;
	
	FPSCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FPSCamera"));
	FPSCamera->SetupAttachment(GetMesh(), TEXT("head"));
	FPSCamera->bUsePawnControlRotation = true;
	FPSCamera->SetRelativeLocation(FVector(6.0f, 25.0f, 0.0f));
	FPSCamera->SetRelativeRotation(FRotator(0.0f, 90.0f, -90.0f));

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom")); 
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; 
	CameraBoom->bUsePawnControlRotation = true; 
	CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 50.0f);

	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); 
	ThirdPersonCamera->bUsePawnControlRotation = false;

	MaskMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MaskMesh"));
	MaskMesh->SetupAttachment(GetMesh(), TEXT("headSocket"));
	
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon"));
	WeaponMesh->SetupAttachment(GetMesh(), TEXT("AxeSocket"));

	CooldownComponent = CreateDefaultSubobject<UT2CooldownComponent>(TEXT("CooldownComponent"));

	DetectionComponent = CreateDefaultSubobject<UT2KillerDetectionComponent>(TEXT("DetectionComponent"));

	FootAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("FootAnchor"));
	FootAnchor->SetupAttachment(GetMesh());
	FootFog = CreateDefaultSubobject<UT2FogComponent>(TEXT("FootFog"));
	FootFog->SetupAttachment(FootAnchor);

	BreathingAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BreathingAudio"));
	BreathingAudioComponent->SetupAttachment(GetMesh(), TEXT("head"));
	BreathingAudioComponent->bAutoActivate = false;

	KillerVisionLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("KillerVisionLight"));
	KillerVisionLight->SetupAttachment(FPSCamera); 

	KillerVisionLight->SetCastShadows(false); 
	KillerVisionLight->Intensity = 5000.f;
}

void AT2KillerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->ViewPitchMin = -55.0f; 
			PC->PlayerCameraManager->ViewPitchMax = 60.0f;  
		}
	}

	if (FPSCamera && ThirdPersonCamera)
	{
		FPSCamera->SetActive(true);
		ThirdPersonCamera->SetActive(false);
		bIsFirstPerson = true;
	}

	if (GetCharacterMovement())
	{
		BaseMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed; 
	}

	if (FPSCamera) 
	{
		DefaultFOV = FPSCamera->FieldOfView;
	}

	if (BreathingSound)
	{
		BreathingAudioComponent->SetSound(BreathingSound);
		BreathingAudioComponent->Play();
        
		if (IsLocallyControlled())
		{
			BreathingAudioComponent->SetVolumeMultiplier(0.5f); 
		}
	}

	if (KillerVisionLight)
	{
		bool bIsMyLocalKiller = IsLocallyControlled();
		KillerVisionLight->SetVisibility(bIsMyLocalKiller);
        
		if (!bIsMyLocalKiller)
		{
			KillerVisionLight->DestroyComponent();
		}
	}
}

void AT2KillerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	float CurrentFOV = FPSCamera->FieldOfView;
	float Target = bIsDashing ? TargetFOV : DefaultFOV;

	float NewFOV = FMath::FInterpTo(CurrentFOV, Target, DeltaTime, 10.0f);
	FPSCamera->SetFieldOfView(NewFOV);
}

void AT2KillerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(AttackAction,	ETriggerEvent::Started, this, &ThisClass::InputAttack);
		EIC->BindAction(LandTrapAction,	ETriggerEvent::Started, this, &ThisClass::HandleLandTrapInput);
		EIC->BindAction(DashAction,		ETriggerEvent::Started,	this, &ThisClass::InputDash);
		EIC->BindAction(ToggleCameraAction, ETriggerEvent::Started, this, &ThisClass::ToggleCameraView);

		EIC->BindAction(WalkAction, ETriggerEvent::Triggered, this, &ThisClass::StartWalk); 
		EIC->BindAction(WalkAction, ETriggerEvent::Completed, this, &ThisClass::EndWalk);
	}
}

void AT2KillerCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AT2KillerCharacter, bIsAttacking);
	DOREPLIFETIME(AT2KillerCharacter, CooldownComponent);
	DOREPLIFETIME(AT2KillerCharacter, bIsWalking);
}

void AT2KillerCharacter::HandleLandTrapInput(const FInputActionValue& InValue)
{
	if (IsLocallyControlled() == true)
	{
		ServerRPCSpawnLandTrap();
	}
}

void AT2KillerCharacter::InputAttack(const FInputActionValue& InValue)
{
	if (bIsAttacking) return;
	if (!IsLocallyControlled()) return;

	ServerAttack();
}

void AT2KillerCharacter::InputDash(const FInputActionValue& InValue)
{
	if (IsLocallyControlled())
	{
		ServerRPCDash();
	}
}

void AT2KillerCharacter::ToggleCameraView(const FInputActionValue& InValue)
{
	if (!IsLocallyControlled()) return;
	
	if (FPSCamera && ThirdPersonCamera)
	{
		if (bIsFirstPerson)
		{
			FPSCamera->SetActive(false);
			ThirdPersonCamera->SetActive(true);

			bUseControllerRotationYaw = false;

			GetCharacterMovement()->bOrientRotationToMovement = true; 
			GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
		}
		else
		{
			ThirdPersonCamera->SetActive(false);
			FPSCamera->SetActive(true);

			bUseControllerRotationYaw = true; 
			
			GetCharacterMovement()->bOrientRotationToMovement = false;
			GetCharacterMovement()->RotationRate = FRotator(0.0f, 3600.0f, 0.0f);
		}
		
		bIsFirstPerson = !bIsFirstPerson;
	}
}

void AT2KillerCharacter::StartWalk()
{
	if (!GetCharacterMovement()) return;
	if (bIsWalking) return; 
	
	if (!IsLocallyControlled()) return;

	bIsWalking = true;  
	UpdateMovementSpeed();

	ServerToggleWalk(true);  
}

void AT2KillerCharacter::EndWalk()
{
	if (!GetCharacterMovement()) return;
	if (!bIsWalking) return;
	
	if (!IsLocallyControlled()) return;

	bIsWalking = false;  
	UpdateMovementSpeed();

	ServerToggleWalk(false);  
}

void AT2KillerCharacter::AttackEnd()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		bIsAttacking = false;
		HitActorsThisAttack.Empty();
	}
}

void AT2KillerCharacter::OnHitSuccessful(APawn* VictimPawn, const FVector& ImpactPoint)
{
	if (!VictimPawn)
	{
		return;
	}

	if (!HasAuthority())
	{
		return;
	}

	if (HitActorsThisAttack.Contains(VictimPawn))
	{
		return;
	}

	HitActorsThisAttack.Add(VictimPawn);
	MulticastRPC_PlayHitSound(ImpactPoint);
}

void AT2KillerCharacter::PlayAttackHitSound(const FVector& Location)
{
	if (AttackHitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AttackHitSound, Location);
	}
}

void AT2KillerCharacter::ResetHitActors()
{
	HitActorsThisAttack.Empty();
}

void AT2KillerCharacter::MulticastPlayAttackMontage_Implementation()
{
	if (!AttackMontage) return;
	
	PlayAnimMontage(AttackMontage);
	
	if (AttackSwingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AttackSwingSound, GetActorLocation());
	}
}

void AT2KillerCharacter::MulticastRPC_PlayHitSound_Implementation(FVector ImpactLocation)
{
	if (AttackSwingSound) 
	{
		UGameplayStatics::PlaySoundAtLocation(this, AttackSwingSound, ImpactLocation);
	}

	if (AttackHitSound) 
	{
		UGameplayStatics::PlaySoundAtLocation(this, AttackHitSound, ImpactLocation);
	}
}

void AT2KillerCharacter::ClientRPC_OnLandTrapSuccess_Implementation()
{
	if (LandTrapSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, LandTrapSound, GetActorLocation());
	}
}

void AT2KillerCharacter::ClientRPC_OnLandTrapDenied_Implementation()
{
}

void AT2KillerCharacter::ClientRPC_OnDashSuccess_Implementation()
{
	if (DashSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DashSound, GetActorLocation());
	}
	if (FootstepSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FootstepSound, GetActorLocation());
	}
}

void AT2KillerCharacter::ClientRPC_OnDashDenied_Implementation()
{
}

void AT2KillerCharacter::OnRep_IsWalking()
{
	UpdateMovementSpeed();
}

void AT2KillerCharacter::ServerToggleWalk_Implementation(bool bNewIsWalking)
{
	bIsWalking = bNewIsWalking;
	UpdateMovementSpeed();
}

void AT2KillerCharacter::UpdateMovementSpeed()
{
	if (!GetCharacterMovement()) return;

	float NewMaxWalkSpeed = bIsWalking ? (BaseMaxWalkSpeed * WalkSpeedMultiplier) : BaseMaxWalkSpeed;
	
	GetCharacterMovement()->MaxWalkSpeed = NewMaxWalkSpeed;
	
	FString RoleStr;
	if (HasAuthority())
		RoleStr = TEXT("SERVER");
	else if (IsLocallyControlled())
		RoleStr = TEXT("CLIENT (Local)");
	else
		RoleStr = TEXT("CLIENT (Remote)");
}

void AT2KillerCharacter::PlayFootstepSound(bool bIsLeftFoot)
{
	if (!FootstepSound) return;
	
	float VolumeToUse = 0.4f; 
	if (bIsWalking)
	{
		VolumeToUse = WalkVolumeMultiplier; 
	}

	const FName FootSocketName = bIsLeftFoot ? FName(TEXT("foot_l")) : FName(TEXT("foot_r"));
    
	FVector FootLocation = GetActorLocation();

	if (GetMesh())
	{
		FootLocation = GetMesh()->GetSocketLocation(FootSocketName);
	}

	UGameplayStatics::SpawnSoundAtLocation(
		this, 
		FootstepSound, 
		FootLocation, 
		FRotator::ZeroRotator, 
		VolumeToUse, 
		1.0f 
	);
}

void AT2KillerCharacter::ServerRPCSpawnLandTrap_Implementation()
{
	if (IsValid(CooldownComponent))
	{
		if (CooldownComponent->GetIsLandTrapOnCooldown())
		{
			ClientRPC_OnLandTrapDenied(); 
			return;
		}
	}
    
	if (IsValid(LandTrapClass))
	{
		FVector SpawnedLocation = (GetActorLocation() + GetActorForwardVector() * 300.f) - FVector(0.f, 0.f, 90.f);
		AKillerLandTrap* SpawnedLandTrap = GetWorld()->SpawnActor<AKillerLandTrap>(LandTrapClass, SpawnedLocation, FRotator::ZeroRotator);

		if (SpawnedLandTrap)
		{
			SpawnedLandTrap->SetOwner(this);
            
			if (IsValid(CooldownComponent))
			{
				CooldownComponent->StartLandTrapCooldown();
			}
            
			ClientRPC_OnLandTrapSuccess();
		}
	}
}

bool AT2KillerCharacter::ServerRPCSpawnLandTrap_Validate()
{
	return true;
}

void AT2KillerCharacter::ServerRPCDash_Implementation()
{
	if (IsValid(CooldownComponent) && CooldownComponent->GetIsDashOnCooldown()) 
	{
		ClientRPC_OnDashDenied(); 
		return;
	}
	
	if (bIsAttacking || bIsDashing) return;

	bIsDashing = true; 
	
	GetCharacterMovement()->BrakingDecelerationWalking = 0.f;

	float DashStrength = 2500.0f; 
	FVector DashVelocity = GetActorForwardVector() * DashStrength;
    
	LaunchCharacter(DashVelocity, true, true);

	GetWorld()->GetTimerManager().SetTimer(DashTimerHandle, [this]()
	{
		bIsDashing = false;
		GetCharacterMovement()->BrakingDecelerationWalking = 2048.f;
	}, DashDuration, false);

	if (IsValid(CooldownComponent))
	{
		CooldownComponent->StartDashCooldown();
	}

	ClientRPC_OnDashSuccess();
}

bool AT2KillerCharacter::ServerRPCDash_Validate()
{
	return true;
}

void AT2KillerCharacter::ServerAttack_Implementation()
{
	if (bIsAttacking || !AttackMontage) return;

	bIsAttacking = true;

	ResetHitActors();
	
	MulticastPlayAttackMontage();
	
	float Duration = AttackMontage->GetPlayLength();
	if (Duration > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			AttackTimerHandle,
			this,
			&AT2KillerCharacter::AttackEnd,
			Duration,
			false
		);
	}
	else
	{
		bIsAttacking = false;
		HitActorsThisAttack.Empty();
	}
}