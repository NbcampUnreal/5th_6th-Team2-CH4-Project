#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AT2SpectatorPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class TEAM02_API AAT2SpectatorPawn : public APawn
{
	GENERATED_BODY()

public:
	AAT2SpectatorPawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// Set spectate target
	UFUNCTION(BlueprintCallable, Category = "Spectator")
	void SetSpectateTarget(AActor* NewTarget);

	// Get spectate target
	UFUNCTION(BlueprintCallable, Category = "Spectator")
	AActor* GetSpectateTarget() const { return SpectateTarget; }

	// Find and set a valid target
	UFUNCTION(BlueprintCallable, Category = "Spectator")
	void FindAndSetTarget();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	// Spectate target
	UPROPERTY()
	TObjectPtr<AActor> SpectateTarget;

	// Camera settings
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float SpringArmLength = 400.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float SpringArmHeightOffset = 100.0f;

	// Mouse sensitivity
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float MouseSensitivity = 1.0f;

	// Pitch limits
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float MinPitch = -60.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float MaxPitch = 60.0f;

private:
	// Mouse input handlers
	void HandleLookYaw(float Value);
	void HandleLookPitch(float Value);

	// Current camera rotation
	float CurrentYaw = 0.0f;
	float CurrentPitch = -30.0f;  // 위에서 내려다보기

	// Log spam prevention
	bool bLoggedTargetInvalid = false;
	float TimeSinceLastTargetSearch = 0.0f;
};
