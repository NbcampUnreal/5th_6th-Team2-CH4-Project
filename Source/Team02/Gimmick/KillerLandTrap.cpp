// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/KillerLandTrap.h"

#include "Character/KillerCharacter/T2KillerCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Net/UnrealNetwork.h"


AKillerLandTrap::AKillerLandTrap()
: bIsExploded(false)
, NetCullDistance(1000.f)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	BoxCollision->SetupAttachment(GetRootComponent());

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(BoxCollision);

	Particle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particle"));
	Particle->SetupAttachment(GetRootComponent());
	Particle->SetAutoActivate(false);

	SetNetCullDistanceSquared(NetCullDistance * NetCullDistance);

	//bAlwaysRelevant = true;
}

void AKillerLandTrap::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() == true)
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Run on server.")), true, true, FLinearColor::Green, 5.f);
	}
	else
	{
		APawn* OwnerPawn = Cast<APawn>(GetOwner());
		if (IsValid(OwnerPawn) == true)
		{
			if (OwnerPawn->IsLocallyControlled() == true)
			{
				UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Run on owning client.")), true, true, FLinearColor::Green, 5.f);
			}
			else
			{
				UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Run on other client.")), true, true, FLinearColor::Green, 5.f);
			}
		}
	}

	if (false == OnActorBeginOverlap.IsAlreadyBound(this, &ThisClass::OnLandTrapBeginOverlap))
	{
		OnActorBeginOverlap.AddDynamic(this, &ThisClass::OnLandTrapBeginOverlap);
	}
}

void AKillerLandTrap::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ADXLandMine::EndPlay()")), true, true, FLinearColor::Green, 5.f);

	if (true == OnActorBeginOverlap.IsAlreadyBound(this, &ThisClass::OnLandTrapBeginOverlap))
	{
		OnActorBeginOverlap.RemoveDynamic(this, &ThisClass::OnLandTrapBeginOverlap);
	}
}

void AKillerLandTrap::OnLandTrapBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (OtherActor && OtherActor->IsA<AT2KillerCharacter>())
	{
		return;
	}
	
	if (HasAuthority() == true)
	{
		if (bIsExploded == true)
		{
			return; 
		}

		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Run on server. Activating One-Time Trap.")), true, true, FLinearColor::Green, 5.f);

		bIsExploded = true;
        
		MulticastRPCSpawnEffect();

		Destroy(); 
	}
	else
	{
		if (bIsExploded == false)
		{
			Particle->Activate(true); 
		}
	}
}

void AKillerLandTrap::MulticastRPCSpawnEffect_Implementation()
{
}

void AKillerLandTrap::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bIsExploded);
}

void AKillerLandTrap::OnRep_IsExploded()
{
	if (true == bIsExploded && IsValid(ExplodedMaterial) == true)
	{
		Mesh->SetMaterial(0, ExplodedMaterial);
	}
}
