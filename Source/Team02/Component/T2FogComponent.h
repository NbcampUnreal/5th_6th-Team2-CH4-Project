// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "T2FogComponent.generated.h"

/**
 * 
 */
UCLASS()
class TEAM02_API UT2FogComponent : public UNiagaraComponent
{
	GENERATED_BODY()

public:
	UT2FogComponent();

	void InitializeFog(USceneComponent* Parent, FName SocketName);

	void SetFogScale(float NewScale);
};
