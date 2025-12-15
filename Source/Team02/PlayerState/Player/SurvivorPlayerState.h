#pragma once

#include "CoreMinimal.h"
#include "PlayerState/T2PlayerState.h"
#include "SurvivorPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHPChanged, float, float)



UCLASS()
class TEAM02_API ASurvivorPlayerState : public AT2PlayerState
{
	GENERATED_BODY()
	
public:
	void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	float MaxHP = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_HP)
	float CurrentHP;

	FOnHPChanged OnHPChanged;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_HP();

	void ApplyDamage(float DamageAmount);

};
