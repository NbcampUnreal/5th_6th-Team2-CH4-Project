#pragma once

#include "CoreMinimal.h"
#include "Character/T2BaseCharacter.h"
#include "T2PlayerCharacter.generated.h"


UCLASS()
class TEAM02_API AT2PlayerCharacter : public AT2BaseCharacter
{
	GENERATED_BODY()
	
public:
	AT2PlayerCharacter();

	virtual void BeginPlay() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	void HandleCrouchInput(const FInputActionValue& InValue);
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> PlayerInputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> CrouchInput;

};
