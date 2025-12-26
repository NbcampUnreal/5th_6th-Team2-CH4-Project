#pragma once

#include "CoreMinimal.h"
#include "GameState/T2GameStateBase.h"
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

    // ========== ��� ���� ���� ==========

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
    int32 RequiredKeys = 6;

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

    // ========== ���� ���� �Լ� ==========
    UPROPERTY(ReplicatedUsing = OnRep_KeyCount, BlueprintReadOnly, Category = "Keys")
    int32 TotalKeyCount = 0;

    UFUNCTION(BlueprintCallable, Category = "Key")
    void AddKeyCount(int32 Amount = 1);

    UFUNCTION()
    void OnRep_KeyCount();

    UFUNCTION(BlueprintCallable, Category = "Keys")
    int32 GetKeyCount() const { return TotalKeyCount; }

    UFUNCTION(BlueprintCallable, Category = "Match")
    int32 GetAliveSurvivorCount() const { return SurvivorsAlive; }

    UFUNCTION(BlueprintCallable, Category = "Match")
    int32 GetEscapedSurvivorCount() const { return SurvivorsEscaped; }



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