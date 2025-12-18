#include "PlayerState/Player/SurvivorPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Character/PlayerCharacter/T2PlayerCharacter.h"
#include "T2PlayGameState.h"
#include "Kismet/GameplayStatics.h"

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
    DOREPLIFETIME(ASurvivorPlayerState, bIsDead);
    DOREPLIFETIME(ASurvivorPlayerState, bIsEscaped);
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

void ASurvivorPlayerState::OnRep_IsDead()
{
    if (bIsDead)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player %s is DEAD"), *GetPlayerName());
        // ��� UI ǥ�� ��
    }
}

void ASurvivorPlayerState::ApplyDamage(float DamageAmount)
{
    if (bIsDead) return;  // �̹� �׾����� ����

    CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.f, MaxHP);
  
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("Client attempting to ApplyDamage! (DENIED)")); 
		return;
	}

	AT2PlayerCharacter* Player = Cast<AT2PlayerCharacter>(GetPawn());

	if (IsValid(Player) == false)
	{
		return;
	}

	if (CurrentHP > 0)
	{
		Player->Multicast_PlayHitMontage();
	}
	else
	{
		Player->OnDeath();
	}
	
	//DEBUGGING LOG
	UE_LOG(LogTemp, Warning, TEXT("SurvivorPS HP: %f"), CurrentHP);

    // HP�� 0�� �Ǹ� ��� ó��
    if (CurrentHP <= 0)
    {
        SetDead();
    }
}

void ASurvivorPlayerState::SetDead()
{
    if (!HasAuthority()) return;
    if (bIsDead) return;

    bIsDead = true;

    // GameState�� �˸�
    if (AT2PlayGameState* GS = GetWorld()->GetGameState<AT2PlayGameState>())
    {
        GS->OnSurvivorDied();
    }

    // ĳ���� ��� ó��
    if (AT2PlayerCharacter* Player = Cast<AT2PlayerCharacter>(GetPawn()))
    {
        Player->OnDeath();
    }
}

void ASurvivorPlayerState::SetEscaped()
{
    if (!HasAuthority()) return;
    if (bIsEscaped || bIsDead) return;

    bIsEscaped = true;

    // GameState�� �˸�
    if (AT2PlayGameState* GS = GetWorld()->GetGameState<AT2PlayGameState>())
    {
        GS->OnSurvivorEscaped();
    }
}