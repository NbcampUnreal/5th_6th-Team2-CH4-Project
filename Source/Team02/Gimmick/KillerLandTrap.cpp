// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/KillerLandTrap.h"

#include "Character/KillerCharacter/T2KillerCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "PlayerState/Player/SurvivorPlayerState.h"


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
}

void AKillerLandTrap::BeginPlay()
{
	Super::BeginPlay();

	if (false == OnActorBeginOverlap.IsAlreadyBound(this, &ThisClass::OnLandTrapBeginOverlap))
	{
		OnActorBeginOverlap.AddDynamic(this, &ThisClass::OnLandTrapBeginOverlap);
	}
}

void AKillerLandTrap::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

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
		if (bIsExploded == true) return; 

		if (APawn* TargetPawn = Cast<APawn>(OtherActor))
		{
			if (ASurvivorPlayerState* SurvivorPS = TargetPawn->GetPlayerState<ASurvivorPlayerState>())
			{
				SurvivorPS->ApplyTrapDebuff(TrapDamage, SpeedReductionMultiplier, SpeedDebuffDuration, VisionDebuffDuration);
			}
		}

		bIsExploded = true;
		OnRep_IsExploded(); 
        
		MulticastRPCSpawnEffect();

		SetLifeSpan(2.0f); 
	}
}

void AKillerLandTrap::MulticastRPCSpawnEffect_Implementation()
{
	if (Particle)
	{
		Particle->Activate(true);
	}

	if (TrapExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, TrapExplosionSound, GetActorLocation());
	}
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
