#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PortalActor.generated.h"

UCLASS()
class TEAM02_API APortalActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APortalActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void ActivatePortal();

	UFUNCTION()
	void OnPlayerEnterPortal(class AT2PlayerCharacter* Player);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	float PortalTimeLimit = 120.0f;

	UFUNCTION(BlueprintPure, Category = "Portal", meta = (WorldContext = "WorldContextObject"))
	static APortalActor* FindActivePortal(const UObject* WorldContextObject);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* PortalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* TriggerVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UNiagaraComponent* PortalEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UPointLightComponent* PortalLight;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal VFX")
	class UNiagaraSystem* PortalNiagaraSystem;

	UPROPERTY(Replicated)
	TArray<APlayerState*> EnteredPlayers;

	UPROPERTY(ReplicatedUsing = OnRep_IsActive)
	bool bIsActive = false;

	UPROPERTY(Replicated)
	float RemainingTime;
	
	UFUNCTION()
	void OnRep_IsActive();
	
	void SpawnPortalEffects();

public:
	UFUNCTION(BlueprintCallable, Category = "Portal")
	bool IsPortalActive() const { return bIsActive; }

	UFUNCTION(BlueprintCallable, Category = "Portal")
	float GetRemainingTime() const { return RemainingTime; }

	UFUNCTION(BlueprintCallable, Category = "Portal")
	int32 GetEnteredPlayersCount() const { return EnteredPlayers.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Portal")
	const TArray<APlayerState*>& GetEnteredPlayers() const { return EnteredPlayers; }

	UFUNCTION(BlueprintPure, Category = "Portal")
	FVector GetPortalLocation() const { return GetActorLocation(); }

protected:
	FTimerHandle PortalTimerHandle;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
		bool bFromSweep, const FHitResult& SweepResult);

	void UpdateTimer();

	void OnPortalTimeExpired();

	bool AreAllSurvivorsEntered();

	void TransitionToNextMap();

	void KillRemainingPlayers();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, Category = "Game")
	FName NextMapName = "NextLevel";

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bShowDebugSphere = true;
};