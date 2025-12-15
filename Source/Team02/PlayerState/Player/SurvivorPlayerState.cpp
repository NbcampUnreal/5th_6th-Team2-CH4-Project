#include "PlayerState/Player/SurvivorPlayerState.h"

void ASurvivorPlayerState::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		CurrentHP = MaxHP;
	}
}

void ASurvivorPlayerState::OnRep_HP()
{
	OnHPChanged.Broadcast(CurrentHP, MaxHP);

	UE_LOG(LogTemp, Warning, TEXT("OnHPChanged.Broadcast(CurrentHP, MaxHP);"));
}

void ASurvivorPlayerState::ApplyDamage(float DamageAmount)
{
	if (!HasAuthority())
	{
		return;
	}

	const float PreviousHP = CurrentHP;

	CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.f, MaxHP);

	if (CurrentHP != PreviousHP)
	{
		OnRep_HP();
	}
}
