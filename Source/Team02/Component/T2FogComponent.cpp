// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/T2FogComponent.h"

#include "T2FogComponent.h"

UT2FogComponent::UT2FogComponent()
{
	bAutoActivate = true;
}

void UT2FogComponent::InitializeFog(USceneComponent* Parent, FName SocketName)
{
	if (Parent)
	{
		this->AttachToComponent(Parent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
        
		this->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.2f)); 
	}
}

void UT2FogComponent::SetFogScale(float NewScale)
{
	this->SetRelativeScale3D(FVector(NewScale));
}