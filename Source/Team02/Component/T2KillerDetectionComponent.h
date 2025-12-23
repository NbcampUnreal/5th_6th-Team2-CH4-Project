// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "T2KillerDetectionComponent.generated.h"


class AT2PlayerCharacter;
class UMaterialInstanceDynamic;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TEAM02_API UT2KillerDetectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:    
	UT2KillerDetectionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;


private:
	TMap<AT2PlayerCharacter*, UMaterialInstanceDynamic*> DetectedSurvivorMaterials;

	UPROPERTY(EditDefaultsOnly, Category = "Detection")
	TObjectPtr<UMaterialInterface> AuraMaterialBase;

	UPROPERTY(EditDefaultsOnly, Category = "Detection")
	FLinearColor AuraColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // 빨강

	UPROPERTY(EditDefaultsOnly, Category = "Detection")
	float AuraIntensity = 3.0f;

	void UpdateDetectedSurvivors();
	void ApplyAuraMaterial(AT2PlayerCharacter* Survivor);
	void RemoveAuraMaterial(AT2PlayerCharacter* Survivor);
};