#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "T2PlayGameState.generated.h"

UENUM(BlueprintType)
enum class EMatchResult : uint8
{
    None,
    KillerWin,
    SurvivorWin,
    Draw
};

UCLASS()
class TEAM02_API AT2PlayGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    AT2PlayGameState();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ========== 경기 진행 상태 ==========

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
    int32 RequiredKeys = 3;

    UPROPERTY(ReplicatedUsing = OnRep_CollectedKeys, BlueprintReadOnly, Category = "Match")
    int32 CollectedKeys = 0;

    UPROPERTY(ReplicatedUsing = OnRep_EscapeGateOpen, BlueprintReadOnly, Category = "Match")
    bool bEscapeGateOpen = false;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
    int32 TotalSurvivors = 4;

    UPROPERTY(ReplicatedUsing = OnRep_SurvivorsAlive, BlueprintReadOnly, Category = "Match")
    int32 SurvivorsAlive = 4;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
    int32 SurvivorsEscaped = 0;

    UPROPERTY(ReplicatedUsing = OnRep_MatchResult, BlueprintReadOnly, Category = "Match")
    EMatchResult MatchResult = EMatchResult::None;

    // ========== 서버 전용 함수 ==========


    UFUNCTION(BlueprintCallable, Category = "Key")
    void AddKeyCount(int32 Amount = 1);


    void AddCollectedKey();

    void OnSurvivorDied();
    void OnSurvivorEscaped();
    void SetEscapeGateOpen(bool bOpen);
    void SetMatchResult(EMatchResult Result);

protected:
    UFUNCTION()
    void OnRep_CollectedKeys();

    UFUNCTION()
    void OnRep_EscapeGateOpen();

    UFUNCTION()
    void OnRep_SurvivorsAlive();

    UFUNCTION()
    void OnRep_MatchResult();
};