// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "PortalActor.generated.h"

UCLASS()
class TEAM02_API APortalActor : public AActor
{
	GENERATED_BODY()
	
public:    
	APortalActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* PortalEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* LightningEffect;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
						bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	TArray<TObjectPtr<APlayerController>> EnteredPlayers;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	bool bIsPortalActive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	float PortalTimeLimit;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	float RemainingTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal|VFX")
	float LightningHeight = 170.f;

	FTimerHandle PortalTimerHandle;
	FTimerHandle CountdownTimerHandle;

	UFUNCTION()
	void OnPortalTimeout();

	UFUNCTION()
	void UpdateCountdown();

	UFUNCTION()
	bool CheckAllPlayersEntered();

	UFUNCTION()
	void TransitionToNextLevel();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FName NextLevelName;

	UFUNCTION(BlueprintCallable, Category = "Portal")
	void ActivatePortal();

	UFUNCTION(BlueprintCallable, Category = "Portal")
	bool HasPlayerEntered(APlayerController* PlayerController) const;

	UFUNCTION(BlueprintPure, Category = "Portal")
	float GetRemainingTime() const { return RemainingTime; }

	UFUNCTION(BlueprintPure, Category = "Portal")
	int32 GetEnteredPlayerCount() const { return EnteredPlayers.Num(); }

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnPortalActivated();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnPlayerEntered(APlayerController* PlayerController);
};