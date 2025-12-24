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

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* PortalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* TriggerVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UParticleSystemComponent* PortalEffect;

	UPROPERTY(Replicated)
	TArray<APlayerState*> EnteredPlayers;

	UPROPERTY(Replicated)
	bool bIsActive = false;

	UPROPERTY(Replicated)
	float RemainingTime;

public:
	UFUNCTION(BlueprintCallable, Category = "Portal")
	bool IsPortalActive() const { return bIsActive; }

	UFUNCTION(BlueprintCallable, Category = "Portal")
	float GetRemainingTime() const { return RemainingTime; }

	UFUNCTION(BlueprintCallable, Category = "Portal")
	int32 GetEnteredPlayersCount() const { return EnteredPlayers.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Portal")
	const TArray<APlayerState*>& GetEnteredPlayers() const { return EnteredPlayers; }

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
};