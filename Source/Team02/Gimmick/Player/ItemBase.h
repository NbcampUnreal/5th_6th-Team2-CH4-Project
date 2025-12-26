#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.generated.h"

class UBoxComponent;
class AT2PlayerCharacter;
class AT2BaseController;

UCLASS()
class TEAM02_API AItemBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AItemBase();

	virtual void BeginInteract(AT2PlayerCharacter* Player);

	virtual void CompleteInteract(AT2PlayerCharacter* Player);

	virtual void CanelInteract(AT2PlayerCharacter* Player);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//virtual void UseItem();

	void DebugDrawCapsule();

	void SetOutlineEnabled(bool bEnable);

	UFUNCTION()
	void OnRep_InteractingPC();

	void HideForLocalPlayer();

	UPROPERTY(VisibleAnywhere)
	bool bIsLevelPlacedActor = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "Item Data")
	FName ItemID = NAME_None;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnOverlapEnd(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> InteractiongBox;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(ReplicatedUsing=OnRep_InteractingPC)
	TObjectPtr<AT2BaseController> InteractingPC;
};
