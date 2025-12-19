#pragma once

#include "CoreMinimal.h"
#include "Gimmick/Player/ItemBase.h"
#include "ExitKey.generated.h"

class UStatickMeshComponent;
class UCapsuleComponent;

UCLASS()
class TEAM02_API AExitKey : public AItemBase
{
	GENERATED_BODY()
	
public:	
	AExitKey();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> SceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> ExitKey;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCapsuleComponent> OverlapCapsule;

	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};
