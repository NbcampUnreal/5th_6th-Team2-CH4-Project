#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlashlightComponent.generated.h"

class USpotLightComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TEAM02_API UFlashlightComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFlashlightComponent();

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY()
	TObjectPtr<USpotLightComponent> CachedSpotLight;

	UPROPERTY(EditDefaultsOnly)
	float MaxBattery = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Battery)
	float CurrentBattery = 100.f;

	UPROPERTY(EditDefaultsOnly)
	float DrainPerSecond = 1.f;

	UPROPERTY(ReplicatedUsing = OnRep_FlashlightOn)
	bool bIsOn = false;

	UFUNCTION()
	void OnRep_Battery();

	UFUNCTION()
	void OnRep_FlashlightOn();

public:
	UFUNCTION(Server, Reliable)
	void Server_ToggleFlashlight();

	void DrainBattery(float DeltaTime);

	void AddBattery();

	
};
