#include "PlayerState/Player/SurvivorPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "UObject/FastReferenceCollector.h"
#include "Character/PlayerCharacter/T2PlayerCharacter.h"

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
	//if (!IsValid(this))
	//{
	//	return;
	//}
	
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

	CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.f, MaxHP);

	if (CurrentHP <= 0)
	{
		if (AT2PlayerCharacter* Player = Cast<AT2PlayerCharacter>(GetPawn()))
		{
			Player->OnDeath();
		}
	}
	
	//DEBUGGING LOG
	UE_LOG(LogTemp, Warning, TEXT("SurvivorPS HP: %f"), CurrentHP);

}
