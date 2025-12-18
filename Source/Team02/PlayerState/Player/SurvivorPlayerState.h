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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HP")
    float MaxHP = 100.f;

    UPROPERTY(ReplicatedUsing = OnRep_HP, BlueprintReadOnly, Category = "HP")
    float CurrentHP;

    FOnHPChanged OnHPChanged;

    void ApplyDamage(float DamageAmount);


    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Status")
    int32 DownCount = 0; 

    // 상태 변경 함수

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    UFUNCTION()
    void OnRep_HP();

};