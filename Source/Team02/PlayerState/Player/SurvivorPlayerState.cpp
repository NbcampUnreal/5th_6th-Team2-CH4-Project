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
    OnHPChanged.Broadcast(CurrentHP, MaxHP);
}

void ASurvivorPlayerState::OnRep_IsDead()
{
    if (bIsDead)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player %s is DEAD"), *GetPlayerName());
        // 사망 UI 표시 등
    }
}

void ASurvivorPlayerState::ApplyDamage(float DamageAmount)
{
    if (!HasAuthority()) return;
    if (bIsDead) return;  // 이미 죽었으면 무시

    CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.f, MaxHP);

    // HP가 0이 되면 사망 처리
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

    // GameState에 알림
    if (AT2PlayGameState* GS = GetWorld()->GetGameState<AT2PlayGameState>())
    {
        GS->OnSurvivorDied();
    }

    // 캐릭터 사망 처리
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

    // GameState에 알림
    if (AT2PlayGameState* GS = GetWorld()->GetGameState<AT2PlayGameState>())
    {
        GS->OnSurvivorEscaped();
    }
}