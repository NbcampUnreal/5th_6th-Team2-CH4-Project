#include "PlayerState/Player/SurvivorPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Character/PlayerCharacter/T2PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "T2PlayGameState.h"
#include "Kismet/GameplayStatics.h"
#include "T2PlayGameMod.h"
#include "T2PlayGameState.h"

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

    CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.f, MaxHP);

    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT("Client attempting to ApplyDamage!  (DENIED)"));
        return;
    }


    OnHPChanged.Broadcast(CurrentHP, MaxHP);
    UE_LOG(LogTemp, Warning, TEXT("Server Broadcast - HP:  %f / %f"), CurrentHP, MaxHP);

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

    UE_LOG(LogTemp, Warning, TEXT("SurvivorPS HP: %f"), CurrentHP);

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

    UE_LOG(LogTemp, Warning, TEXT("=== SetDead Called ==="));

    // 1. GameState에 알림 (SurvivorsAlive 감소)
    AT2PlayGameState* GS = GetWorld()->GetGameState<AT2PlayGameState>();
    if (GS)
    {
        GS->OnSurvivorDied();
        UE_LOG(LogTemp, Warning, TEXT("GameState->OnSurvivorDied called.  SurvivorsAlive:  %d"), GS->SurvivorsAlive);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameState is NULL in SetDead!"));
    }

    // 2. GameMode에 알림 (승패 체크)
    AT2PlayGameMod* GM = GetWorld()->GetAuthGameMode<AT2PlayGameMod>();
    if (GM)
    {
        GM->OnPlayerDied(nullptr);
        UE_LOG(LogTemp, Warning, TEXT("GameMode->OnPlayerDied called"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameMode is NULL in SetDead! "));
    }

    // 3. 캐릭터 사망 처리
    if (APawn* MyPawn = GetPawn())
    {
        // 사망 애니메이션 등
        UE_LOG(LogTemp, Warning, TEXT("Pawn death processing... "));
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