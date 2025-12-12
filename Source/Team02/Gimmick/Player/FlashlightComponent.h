#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlashlightComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TEAM02_API UFlashlightComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFlashlightComponent();

	virtual void BeginPlay() override;

	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxBattery = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CurrentBattery = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DrainPerSecond = 1.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float bIsOn = false;

public:
	void ToggleFlashlight();

	void DrainBattery(float DeltaTime);

	void AddBattery();
};
