// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "T2CooldownComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TEAM02_API UT2CooldownComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UT2CooldownComponent();

	UFUNCTION(BlueprintCallable, Category = "Cooldown")
	void StartLandTrapCooldown();
    
	UFUNCTION(BlueprintCallable, Category = "Cooldown")
	void StartDashCooldown();

	UFUNCTION(BlueprintPure, Category = "Cooldown")
	float GetLandTrapCooldownProgress() const;
    
	UFUNCTION(BlueprintPure, Category = "Cooldown")
	float GetDashCooldownProgress() const;

	UFUNCTION(BlueprintPure, Category = "Cooldown")
	FORCEINLINE bool GetIsLandTrapOnCooldown() const { return bIsLandTrapOnCooldown; }

	UFUNCTION(BlueprintPure, Category = "Cooldown")
	FORCEINLINE bool GetIsDashOnCooldown() const { return bIsDashOnCooldown; }
    
private:
	UFUNCTION()
	void ClearLandTrapCooldown();
	UFUNCTION()
	void ClearDashCooldown();
    
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LandTrap")
	float LandTrapCooldownDuration = 15.0f;
    
	UPROPERTY(Replicated) 
	float LandTrapCooldownStartTime = 0.0f;
    
	UPROPERTY(Replicated) 
	bool bIsLandTrapOnCooldown = false;
	
	FTimerHandle LandTrapCooldownTimerHandle;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash")
	float DashCooldownDuration = 5.0f; 
    
	UPROPERTY(Replicated)
	float DashCooldownStartTime = 0.0f;
    
	UPROPERTY(Replicated)
	bool bIsDashOnCooldown = false;
	
	FTimerHandle DashCooldownTimerHandle;

private:
	UFUNCTION()
	void OnRep_LandTrapCooldownStartTime(); 
    
	UFUNCTION()
	void OnRep_DashCooldownStartTime();
    
};