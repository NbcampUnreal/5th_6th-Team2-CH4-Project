#include "PlayerState/Player/SurvivorPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Character/PlayerCharacter/T2PlayerCharacter.h"
#include "T2PlayGameState.h"
#include "Kismet/GameplayStatics.h"
#include "T2PlayGameMod.h"

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
    DOREPLIFETIME(ASurvivorPlayerState, DownCount);
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

    CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.f, MaxHP);

	AT2PlayerCharacter* Player = Cast<AT2PlayerCharacter>(GetPawn());

	if (IsValid(Player) == false) return;

	if (CurrentHP <= 0.f && bIsAlive)
	{
		bIsAlive = false;

		Player->OnDeath();

		if (AT2PlayGameMod* GM = GetWorld()->GetAuthGameMode<AT2PlayGameMod>())
		{
			GM->OnSurvivorDied();
		}
	}

	else
	{
		Player->Multicast_PlayHitMontage();
	}
	//DEBUGGING LOG
	UE_LOG(LogTemp, Warning, TEXT("SurvivorPS HP: %f"), CurrentHP);
}


