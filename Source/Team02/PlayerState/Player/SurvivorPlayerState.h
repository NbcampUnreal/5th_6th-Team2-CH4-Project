#pragma once

#include "CoreMinimal.h"
#include "PlayerState/T2PlayerState.h"
#include "SurvivorPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnVisionDebuffChanged, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHPChanged, float, float);

UCLASS()
class TEAM02_API ASurvivorPlayerState : public AT2PlayerState
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HP")
    float MaxHP = 100.f;

    UPROPERTY(ReplicatedUsing = OnRep_HP, BlueprintReadOnly, Category = "HP")
    float CurrentHP;

    FOnHPChanged OnHPChanged;

    void ApplyDamage(float DamageAmount);

    UPROPERTY(ReplicatedUsing = OnRep_IsDead, BlueprintReadOnly, Category = "Status")
    bool bIsDead = false;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Status")
    bool bIsEscaped = false;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Status")
    int32 DownCount = 0;  

    void SetEscaped();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    UFUNCTION()
    void OnRep_HP();

    UFUNCTION()
    void OnRep_IsDead();

#pragma region Debuffed
public:
    void ApplyTrapDebuff(float Damage, float SpeedMult, float SpeedDur, float VisionDur);

    FOnVisionDebuffChanged OnVisionDebuffChanged;

    float GetSpeedMultiplier() const { return bIsSpeedDebuffed ? CurrentSpeedMultiplier : 1.0f; }

protected:
    UPROPERTY(ReplicatedUsing = OnRep_UpdateSpeed)
    bool bIsSpeedDebuffed = false;
    
    float CurrentSpeedMultiplier = 1.0f;

    UPROPERTY(ReplicatedUsing = OnRep_VisionDebuff)
    bool bIsVisionDebuffed = false;

    FTimerHandle SpeedDebuffTimerHandle;
    FTimerHandle VisionDebuffTimerHandle;

    void ResetSpeedDebuff();
    void ResetVisionDebuff();

    UFUNCTION()
    void OnRep_UpdateSpeed();

    UFUNCTION()
    void OnRep_VisionDebuff();

#pragma endregion
};