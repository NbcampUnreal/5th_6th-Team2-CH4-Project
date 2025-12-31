// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/T2CooldownComponent.h"

#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

UT2CooldownComponent::UT2CooldownComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UT2CooldownComponent::StartLandTrapCooldown()
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || bIsLandTrapOnCooldown) return;

	if (UWorld* World = GetWorld())
	{
		bIsLandTrapOnCooldown = true;
		LandTrapCooldownStartTime = World->GetTimeSeconds();

		World->GetTimerManager().SetTimer(
			LandTrapCooldownTimerHandle,
			this,
			&ThisClass::ClearLandTrapCooldown,
			LandTrapCooldownDuration,
			false
		);
	}
}

void UT2CooldownComponent::StartDashCooldown()
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || bIsDashOnCooldown) return;

	if (UWorld* World = GetWorld())
	{
		bIsDashOnCooldown = true;
		DashCooldownStartTime = World->GetTimeSeconds();

		World->GetTimerManager().SetTimer(
			DashCooldownTimerHandle,
			this,
			&ThisClass::ClearDashCooldown,
			DashCooldownDuration,
			false
		);
	}
}

float UT2CooldownComponent::GetLandTrapCooldownProgress() const
{
	if (!bIsLandTrapOnCooldown)
	{
		return 1.0f; 
	}

	if (UWorld* World = GetWorld())
	{
		float CurrentTime = World->GetTimeSeconds();
		float ElapsedTime = CurrentTime - LandTrapCooldownStartTime;
        
		float Progress = ElapsedTime / LandTrapCooldownDuration; 

		return FMath::Clamp(Progress, 0.0f, 1.0f);
	}
    
	return 0.0f;
}

float UT2CooldownComponent::GetDashCooldownProgress() const
{
	if (!bIsDashOnCooldown)
	{
		return 1.0f;
	}

	if (UWorld* World = GetWorld())
	{
		float CurrentTime = World->GetTimeSeconds();
		float ElapsedTime = CurrentTime - DashCooldownStartTime;
        
		float Progress = ElapsedTime / DashCooldownDuration;

		return FMath::Clamp(Progress, 0.0f, 1.0f);
	}
    
	return 0.0f;
}

void UT2CooldownComponent::ClearLandTrapCooldown()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	bIsLandTrapOnCooldown = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LandTrapCooldownTimerHandle);
	}
}

void UT2CooldownComponent::ClearDashCooldown()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	bIsDashOnCooldown = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DashCooldownTimerHandle);
	}
}

void UT2CooldownComponent::OnRep_LandTrapCooldownStartTime()
{
}

void UT2CooldownComponent::OnRep_DashCooldownStartTime()
{
}

void UT2CooldownComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UT2CooldownComponent, bIsLandTrapOnCooldown, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UT2CooldownComponent, LandTrapCooldownStartTime, COND_OwnerOnly);
	
	DOREPLIFETIME_CONDITION(UT2CooldownComponent, bIsDashOnCooldown, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UT2CooldownComponent, DashCooldownStartTime, COND_OwnerOnly);

}

