#include "PlayerState/Player/SurvivorPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

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
