#pragma once

#include "CoreMinimal.h"
#include "PlayerState/T2PlayerState.h"
#include "SurvivorPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHPChanged, float, float);

UCLASS()
class TEAM02_API ASurvivorPlayerState : public AT2PlayerState
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    // ========== HP 시스템 ==========
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HP")
    float MaxHP = 100.f;

    UPROPERTY(ReplicatedUsing = OnRep_HP, BlueprintReadOnly, Category = "HP")
    float CurrentHP;

    FOnHPChanged OnHPChanged;

    void ApplyDamage(float DamageAmount);

    // ========== 생존자 상태 (추가) ==========
    UPROPERTY(ReplicatedUsing = OnRep_IsDead, BlueprintReadOnly, Category = "Status")
    bool bIsDead = false;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Status")
    bool bIsEscaped = false;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Status")
    int32 DownCount = 0;  // 다운 횟수 (선택적)

    // 상태 변경 함수
    void SetDead();
    void SetEscaped();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    UFUNCTION()
    void OnRep_HP();

    UFUNCTION()
    void OnRep_IsDead();
};