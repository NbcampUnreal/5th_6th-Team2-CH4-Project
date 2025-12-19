// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KillerLandTrap.generated.h"

class USceneComponent;
class UBoxComponent;
class UStaticMeshComponent;
class UParticleSystemComponent;
class UMaterial;

UCLASS()
class TEAM02_API AKillerLandTrap : public AActor
{
	GENERATED_BODY()
	
public:	
	AKillerLandTrap();

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnLandTrapBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCSpawnEffect();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_IsExploded();

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess))
	TObjectPtr<UBoxComponent> BoxCollision;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess))
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess))
	TObjectPtr<UParticleSystemComponent> Particle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap Settings", Meta = (AllowPrivateAccess))
	TObjectPtr<USoundBase> TrapExplosionSound;

	UPROPERTY(ReplicatedUsing = OnRep_IsExploded)
	uint8 bIsExploded : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess))
	float NetCullDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess))
	TObjectPtr<UMaterial> ExplodedMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap Settings", Meta = (AllowPrivateAccess))
	float TrapDamage = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap Settings", Meta = (AllowPrivateAccess))
	float SpeedReductionMultiplier = 0.5f;  

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap Settings", Meta = (AllowPrivateAccess))
	float SpeedDebuffDuration = 8.f;  

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap Settings", Meta = (AllowPrivateAccess))
	float VisionDebuffDuration = 5.f; 
};