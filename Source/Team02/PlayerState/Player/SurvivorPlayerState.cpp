#include "PlayerState/Player/SurvivorPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "UObject/FastReferenceCollector.h"

void ASurvivorPlayerState::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		CurrentHP = MaxHP;
	}
}

void ASurvivorPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASurvivorPlayerState, CurrentHP);
}

void ASurvivorPlayerState::OnRep_HP()
{
	if (!IsValid(this))
	{
		return;
	}
	
	OnHPChanged.Broadcast(CurrentHP, MaxHP);

	UE_LOG(LogTemp, Warning, TEXT("OnHPChanged.Broadcast(CurrentHP, MaxHP);"));
}

void ASurvivorPlayerState::ApplyDamage(float DamageAmount)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("Client attempting to ApplyDamage! (DENIED)")); 
		return;
	}

	const float PreviousHP = CurrentHP;

	CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.f, MaxHP);
	
	//DEBUGGING LOG
	UE_LOG(LogTemp, Warning, TEXT("DAMAGE APPLIED (Server): %s new HP: %f"), *GetOwner()->GetName(), CurrentHP);

	if (CurrentHP != PreviousHP)
	{
		OnRep_HP();
	}
}
