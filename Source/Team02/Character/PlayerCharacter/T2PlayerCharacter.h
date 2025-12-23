#pragma once

#include "CoreMinimal.h"
#include "Character/T2BaseCharacter.h"
#include "T2PlayerCharacter.generated.h"


class UFlashlightComponent;
class USpringArmComponent;
class UCamera;
class USpotLightComponent;
class AItemBase;
class UUserWidget;
class UMaterialInstanceDynamic;

UCLASS()
class TEAM02_API AT2PlayerCharacter : public AT2BaseCharacter
{
	GENERATED_BODY()
	
public:
	AT2PlayerCharacter();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void OnRep_PlayerState() override;

	void HandleCrouchInput(const FInputActionValue& InValue);

	void HandleFlashlightInput();

	UFUNCTION(Server, Reliable)
	void Server_ToggleCrouch();

	void HandleHPChanged(float CurrentHP, float MaxHP);

	float TakeDamage(float DamageAmount,FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void OnDeath();

	void OnDeathMontageEneded();

	void SetInteractableItem(AItemBase* Item);

	void ClearInteractableItem(AItemBase* Item);

	void OnInteractStart();

	void OnInteractCompleted();

	void OnInteractCanceled();

	UFUNCTION(Server, Reliable)
	void Server_BeginInteract(AItemBase* Item);

	UFUNCTION(Server, Reliable)
	void Server_CompleteInteract(AItemBase* Item);

	UFUNCTION(Server, Reliable)
	void Server_CancelInteract(AItemBase* Item);

	void AddNearbyItem(AItemBase* Item);

	void RemoveNearbyItem(AItemBase* Item);

	void UpdateInteractTarget();


public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCameraComponent> ThirdPersonCamera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> FirstPersonArms;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> FP_ArmPivot;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> FlashlightMesh;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly)
	TObjectPtr<UFlashlightComponent> FlashlightComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight")
	TObjectPtr<USpotLightComponent> Flashlight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> PlayerInputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> CrouchInput;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractInput;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TObjectPtr<AItemBase> CurrentInteractItem;

	UPROPERTY()
	TSet<TObjectPtr<AItemBase>> NearbyItems;

protected:
	float CurrentInteractTime = 0.f;
	float RequiredInteractTime = 1.5f;

	UPROPERTY(Replicated)
	bool bIsInteracting = false;

	FTimerHandle InteractTraceTimer;

#pragma region TEST
public:

	void HandleViewModeInput(const FInputActionValue& InValue);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ViewModeInput;

	bool bIsFirstPerson = false;

#pragma endregion

};
