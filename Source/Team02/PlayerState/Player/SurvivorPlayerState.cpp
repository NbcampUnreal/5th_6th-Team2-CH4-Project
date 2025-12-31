#include "PlayerState/Player/SurvivorPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Character/PlayerCharacter/T2PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "T2PlayGameState.h"  
#include "Kismet/GameplayStatics.h"
#include "PlayerState/Player/InventoryComponent.h"

ASurvivorPlayerState::ASurvivorPlayerState()
{
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	bReplicates = true;

}

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
	DOREPLIFETIME(ASurvivorPlayerState, CurrentSpeedMultiplier);  
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
    
	OnVisionDebuffChanged.Broadcast(true);
	UE_LOG(LogTemp, Warning, TEXT("Trap Debuff Applied - Speed: %.2f, SpeedDur: %.1f, VisionDur: %.1f"), SpeedMult, SpeedDur, VisionDur);
}

void ASurvivorPlayerState::ResetSpeedDebuff()
{
	bIsSpeedDebuffed = false;
	CurrentSpeedMultiplier = 1.0f;
	OnRep_UpdateSpeed(); 
    
	UE_LOG(LogTemp, Warning, TEXT("Speed Debuff Reset"));
}

void ASurvivorPlayerState::ResetVisionDebuff()
{
	bIsVisionDebuffed = false;
	OnRep_VisionDebuff(); 
    
	UE_LOG(LogTemp, Warning, TEXT("Vision Debuff Reset"));
}

void ASurvivorPlayerState::OnRep_UpdateSpeed()
{
	if (AT2PlayerCharacter* Player = Cast<AT2PlayerCharacter>(GetPawn()))
	{
		if (UCharacterMovementComponent* MoveComp = Player->GetCharacterMovement()) 
		{
			float BaseSpeed = 300.0f;
			float NewSpeed = bIsSpeedDebuffed ? (BaseSpeed * CurrentSpeedMultiplier) : BaseSpeed;
			MoveComp->MaxWalkSpeed = NewSpeed;
            
			FString RoleStr = HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT");
			UE_LOG(LogTemp, Warning, TEXT("%s: Speed Updated to %.0f (Debuffed: %s, Multiplier: %.2f)"), 
				*RoleStr, NewSpeed, bIsSpeedDebuffed ? TEXT("TRUE") : TEXT("FALSE"), CurrentSpeedMultiplier);
		}
	}
}

void ASurvivorPlayerState::OnRep_VisionDebuff()
{
	OnVisionDebuffChanged.Broadcast(bIsVisionDebuffed);
    
	OnDetectionStatusChanged.Broadcast(bIsVisionDebuffed);
    
	UE_LOG(LogTemp, Warning, TEXT("OnRep_VisionDebuff - Status: %s"), bIsVisionDebuffed ? TEXT("DETECTED") : TEXT("HIDDEN"));
}

void ASurvivorPlayerState::ApplyDamage(float DamageAmount)
{
	if (bIsDead) return;

	CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.f, MaxHP);

	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("Client attempting to ApplyDamage! (DENIED)"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Server Broadcast - HP: %f / %f"), CurrentHP, MaxHP);

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
}

void ASurvivorPlayerState::ApplyHealByItem(float HealAmount)
{
	if (!HasAuthority()) return;

	CurrentHP = FMath::Clamp(CurrentHP + HealAmount, 0.f, MaxHP);

	UE_LOG(LogTemp, Warning, TEXT("SurvivorPS HP: %f"), CurrentHP);
}

void ASurvivorPlayerState::SetEscaped()
{
	if (!HasAuthority()) return;
	if (bIsEscaped || bIsDead) return;

	bIsEscaped = true;

	if (AT2PlayGameState* GS = GetWorld()->GetGameState<AT2PlayGameState>())
	{
		GS->OnSurvivorEscaped();
	}
}