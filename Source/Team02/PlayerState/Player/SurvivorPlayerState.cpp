#include "PlayerState/Player/SurvivorPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Character/PlayerCharacter/T2PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
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

	DOREPLIFETIME(ASurvivorPlayerState, bIsSpeedDebuffed);
	DOREPLIFETIME(ASurvivorPlayerState, bIsVisionDebuffed);
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

void ASurvivorPlayerState::ApplyTrapDebuff(float Damage, float SpeedMult, float SpeedDur, float VisionDur)
{
	if (!HasAuthority() || bIsDead) return;

	ApplyDamage(Damage);

	bIsSpeedDebuffed = true;
	CurrentSpeedMultiplier = SpeedMult;
	OnRep_UpdateSpeed();
    
	GetWorldTimerManager().SetTimer(SpeedDebuffTimerHandle, this, &ASurvivorPlayerState::ResetSpeedDebuff, SpeedDur, false);

	bIsVisionDebuffed = true;
	GetWorldTimerManager().SetTimer(VisionDebuffTimerHandle, this, &ASurvivorPlayerState::ResetVisionDebuff, VisionDur, false);
	OnRep_VisionDebuff();
}

void ASurvivorPlayerState::ResetSpeedDebuff()
{
	bIsSpeedDebuffed = false;
	CurrentSpeedMultiplier = 1.0f;
	OnRep_UpdateSpeed();
}

void ASurvivorPlayerState::ResetVisionDebuff()
{
	bIsVisionDebuffed = false;
	OnRep_VisionDebuff();
}

void ASurvivorPlayerState::OnRep_UpdateSpeed()
{
	if (AT2PlayerCharacter* Player = Cast<AT2PlayerCharacter>(GetPawn()))
	{
		if (UCharacterMovementComponent* MoveComp = Player->GetCharacterMovement())
		{
			float BaseSpeed = 300.0f;
			MoveComp->MaxWalkSpeed = bIsSpeedDebuffed ? (BaseSpeed * CurrentSpeedMultiplier) : BaseSpeed;
		}
	}
}

void ASurvivorPlayerState::OnRep_VisionDebuff()
{
	OnVisionDebuffChanged.Broadcast(bIsVisionDebuffed);
	//UI
}

void ASurvivorPlayerState::ApplyDamage(float DamageAmount)
{
    if (bIsDead) return; 
  
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("Client attempting to ApplyDamage! (DENIED)")); 
		return;
	}

	CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.f, MaxHP);

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