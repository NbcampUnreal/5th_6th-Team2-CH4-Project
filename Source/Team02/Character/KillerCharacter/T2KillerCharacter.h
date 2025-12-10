// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/T2BaseCharacter.h"
#include "T2KillerCharacter.generated.h"

class UCameraComponent;
class UStaticMeshComponent;
class UT2CooldownComponent;
class UUW_KillerHUD;

/**
 * 
 */
UCLASS()
class TEAM02_API AT2KillerCharacter : public AT2BaseCharacter
{
	GENERATED_BODY()

	public:
	AT2KillerCharacter();

	virtual void BeginPlay() override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region Killer Character
public:

	FORCEINLINE UCameraComponent* GetCamera() const { return FPSCamera; }
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* MaskMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FPSCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UT2CooldownComponent> CooldownComponent;

#pragma endregion

#pragma region Killer Input
protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> LandTrapAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> DashAction;
	
	void HandleLandTrapInput(const FInputActionValue& InValue);
	void InputAttack(const FInputActionValue& InValue);
	void InputDash(const FInputActionValue& InValue);
#pragma endregion

#pragma region Attack System
public:
	UFUNCTION(BlueprintCallable) 
	void AttackEnd();

protected:
	UPROPERTY(EditDefaultsOnly, Category="Attack")
	UAnimMontage* AttackMontage;

	UPROPERTY(ReplicatedUsing = OnRep_IsAttacking)
	bool bIsAttacking = false;

	FTimerHandle AttackTimerHandle;

	UFUNCTION()
	void OnRep_IsAttacking();

	UFUNCTION(Server, Reliable)
	void ServerAttack();
#pragma endregion
	
#pragma region LandTrap System
private:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCSpawnLandTrap();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="LandTrap")
	TSubclassOf<AActor> LandTrapClass;

#pragma endregion

#pragma region Dash System;
protected:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCDash();

protected:
	UPROPERTY(EditAnywhere, Category = "Dash")
	float DashImpulseStrength = 2000.0f;

#pragma endregion 
};
