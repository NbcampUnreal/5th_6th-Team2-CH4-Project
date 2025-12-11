// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/KillerCharacter/T2KillerCharacter.h"

#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Gimmick/KillerLandTrap.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Component/T2CooldownComponent.h"

AT2KillerCharacter::AT2KillerCharacter()
{
	FPSCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FPSCamera"));
	FPSCamera->SetupAttachment(GetMesh(), TEXT("head"));
	FPSCamera->bUsePawnControlRotation = true;
	FPSCamera->SetRelativeLocation(FVector(6.0f, 25.0f, 0.0f));
	FPSCamera->SetRelativeRotation(FRotator(0.0f, 90.0f, -90.0f));

	MaskMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MaskMesh"));
	MaskMesh->SetupAttachment(GetMesh(), TEXT("headSocket"));
	
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon"));
	WeaponMesh->SetupAttachment(GetMesh(), TEXT("AxeSocket"));

	CooldownComponent = CreateDefaultSubobject<UT2CooldownComponent>(TEXT("CooldownComponent"));
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
}

void AT2KillerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(AttackAction,	ETriggerEvent::Started, this, &ThisClass::InputAttack);
		EIC->BindAction(LandTrapAction,	ETriggerEvent::Started, this, &ThisClass::HandleLandTrapInput);
		EIC->BindAction(DashAction,		ETriggerEvent::Started,	this, &ThisClass::InputDash);
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
	
	if (bIsAttacking) return;

	float DashStrength = 1000.0f;
	FVector DashVelocity = GetActorForwardVector() * DashStrength;
	DashVelocity.Z = 200.0f;

	LaunchCharacter(DashVelocity, true, true);

	UKismetSystemLibrary::PrintString(this, TEXT("Dash!"), true, true, FLinearColor::Blue, 2.f);

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
