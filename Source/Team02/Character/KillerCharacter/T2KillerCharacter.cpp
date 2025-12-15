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
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

AT2KillerCharacter::AT2KillerCharacter()
{
	
	
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

	FootAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("FootAnchor"));
	FootAnchor->SetupAttachment(GetMesh());
	FootFog = CreateDefaultSubobject<UT2FogComponent>(TEXT("FootFog"));
	FootFog->SetupAttachment(FootAnchor);
	
}

void AT2KillerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC -> PlayerCameraManager)
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

	if (FPSCamera) DefaultFOV = FPSCamera->FieldOfView;
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
}

void AT2KillerCharacter::HandleLandTrapInput(const FInputActionValue& InValue)
{
	if (IsLocallyControlled() == true)
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("HandleLandTrapInput()")), true, true, FLinearColor::Green, 5.f);
		ServerRPCSpawnLandTrap();
	}
}

void AT2KillerCharacter::InputAttack(const FInputActionValue& InValue)
{
	UE_LOG(LogTemp, Warning, TEXT("InputAttack called"));

	if (bIsAttacking) return;
	if (!IsLocallyControlled()) return;

	ServerAttack();
}

void AT2KillerCharacter::InputDash(const FInputActionValue& InValue)
{
	UE_LOG(LogTemp, Warning, TEXT("InputDash called"));
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
			
			UKismetSystemLibrary::PrintString(this, TEXT("3rd Person View"), true, true, FLinearColor::Yellow, 2.f);
		}
		else
		{
			ThirdPersonCamera->SetActive(false);
			FPSCamera->SetActive(true);

			bUseControllerRotationYaw = true; 
			
			GetCharacterMovement()->bOrientRotationToMovement = false;
			GetCharacterMovement()->RotationRate = FRotator(0.0f, 3600.0f, 0.0f);
			
			UKismetSystemLibrary::PrintString(this, TEXT("1st Person View"), true, true, FLinearColor::Yellow, 2.f);
		}
		
		bIsFirstPerson = !bIsFirstPerson;
	}
}

void AT2KillerCharacter::StartWalk()
{
	if (!GetCharacterMovement() || bIsWalking) return;

	bIsWalking = true;
	GetCharacterMovement()->MaxWalkSpeed = BaseMaxWalkSpeed * WalkSpeedMultiplier;

	UKismetSystemLibrary::PrintString(this, TEXT("Walk Started"), true, true, FLinearColor::Blue, 1.f);
}

void AT2KillerCharacter::EndWalk()
{
	if (!GetCharacterMovement() || !bIsWalking) return;

	bIsWalking = false;
	GetCharacterMovement()->MaxWalkSpeed = BaseMaxWalkSpeed;
	
	UKismetSystemLibrary::PrintString(this, TEXT("Walk Ended (Run)"), true, true, FLinearColor::Blue, 1.f);
}

void AT2KillerCharacter::AttackEnd()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		bIsAttacking = false;
		UE_LOG(LogTemp, Warning, TEXT("Attack Ended, bIsAttacking reset to false."));
	}
}

void AT2KillerCharacter::OnRep_IsAttacking()
{
	if (bIsAttacking && AttackMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnRep_IsAttacking: play montage on %s"),
		   HasAuthority() ? TEXT("Server") : TEXT("Client"));

		PlayAnimMontage(AttackMontage);
	}
}



void AT2KillerCharacter::PlayFootstepSound(bool bIsLeftFoot)
{
	if (!FootstepSound) return;
	
	
	float VolumeToUse = 0.7f; 
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

	FString FootstepType = bIsWalking ? TEXT("Walk") : TEXT("Run");
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Footstep: %s (Vol: %.2f) at %s"), *FootstepType, VolumeToUse, *FootSocketName.ToString()), true, true, FLinearColor::White, 0.5f);

}

void AT2KillerCharacter::ServerRPCSpawnLandTrap_Implementation()
{

	if (IsValid(CooldownComponent))
	{
		if (CooldownComponent->GetIsLandTrapOnCooldown())
		{
			UKismetSystemLibrary::PrintString(this, TEXT("LandTrap Cooldown Active!"), true, true, FLinearColor::Red, 2.f);
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
		}
	}

	if (LandTrapSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, LandTrapSound, GetActorLocation());
	}
}

bool AT2KillerCharacter::ServerRPCSpawnLandTrap_Validate()
{
	return true;
}


void AT2KillerCharacter::ServerRPCDash_Implementation()
{
	if (IsValid(CooldownComponent))
	{
		if (CooldownComponent->GetIsDashOnCooldown()) 
		{
			UKismetSystemLibrary::PrintString(this, TEXT("Dash Cooldown Active!"), true, true, FLinearColor::Red, 2.f);
			return;
		}
	}
	
	if (bIsAttacking || bIsDashing) return;

	bIsDashing = true;
	
	if (DashSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DashSound, GetActorLocation());
	}

	GetCharacterMovement()->BrakingDecelerationWalking = 0.f;

	float DashStrength = 2500.0f; 
	FVector DashVelocity = GetActorForwardVector() * DashStrength;
    
	LaunchCharacter(DashVelocity, true, true);

	GetWorld()->GetTimerManager().SetTimer(DashTimerHandle, [this]()
	{
		bIsDashing = false;
		GetCharacterMovement()->BrakingDecelerationWalking = 2048.f;
	}, DashDuration, false);

	if (FootstepSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FootstepSound, GetActorLocation());
	}
	if (IsValid(CooldownComponent))
	{
		CooldownComponent->StartDashCooldown();
	}
	
}

bool AT2KillerCharacter::ServerRPCDash_Validate()
{
	return true;
}

void AT2KillerCharacter::ServerAttack_Implementation()
{
	if (bIsAttacking || !AttackMontage) return;

	bIsAttacking = true;
	float Duration = PlayAnimMontage(AttackMontage);
	
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
	}
}
