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

    // 열쇠 시스템 (나중에 사용)
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
    int32 RequiredKeys = 3;

    UPROPERTY(ReplicatedUsing = OnRep_CollectedKeys, BlueprintReadOnly, Category = "Match")
    int32 CollectedKeys = 0;

    // 탈출문 상태
    UPROPERTY(ReplicatedUsing = OnRep_EscapeGateOpen, BlueprintReadOnly, Category = "Match")
    bool bEscapeGateOpen = false;

    // 생존자 상태
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
    int32 TotalSurvivorCount = 4;

    UPROPERTY(ReplicatedUsing = OnRep_SurvivorsAlive, BlueprintReadOnly, Category = "Match")
    int32 AliveSurvivorCount = 4;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
    int32 SurvivorsEscaped = 0;

    // 경기 결과
    UPROPERTY(ReplicatedUsing = OnRep_MatchResult, BlueprintReadOnly, Category = "Match")
    EMatchResult MatchResult = EMatchResult::None;

    // ========== 서버 전용 함수 ==========

    void AddCollectedKey();
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