#include "T2PlayGameState.h"
#include "Net/UnrealNetwork.h"

AT2PlayGameState::AT2PlayGameState()
{
}

void AT2PlayGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AT2PlayGameState, RequiredKeys);
    DOREPLIFETIME(AT2PlayGameState, CollectedKeys);
    DOREPLIFETIME(AT2PlayGameState, bEscapeGateOpen);
    DOREPLIFETIME(AT2PlayGameState, TotalSurvivorCount);
    DOREPLIFETIME(AT2PlayGameState, AliveSurvivorCount);
    DOREPLIFETIME(AT2PlayGameState, SurvivorsEscaped);
    DOREPLIFETIME(AT2PlayGameState, MatchResult);
}

void AT2PlayGameState::AddCollectedKey()
{
    if (GetLocalRole() != ROLE_Authority) return;

    CollectedKeys++;
    OnRep_CollectedKeys();

    // 열쇠 다 모으면 탈출문 열림
    if (CollectedKeys >= RequiredKeys)
    {
        SetEscapeGateOpen(true);
    }
}

void AT2PlayGameState::SetEscapeGateOpen(bool bOpen)
{
    if (GetLocalRole() != ROLE_Authority) return;

    bEscapeGateOpen = bOpen;
}

void AT2PlayGameState::SetMatchResult(EMatchResult Result)
{
    if (GetLocalRole() != ROLE_Authority) return;

    MatchResult = Result;
}

void AT2PlayGameState::OnRep_CollectedKeys()
{
    // UI 업데이트 등
    UE_LOG(LogTemp, Warning, TEXT("Keys Collected: %d / %d"), CollectedKeys, RequiredKeys);
}

void AT2PlayGameState::OnRep_EscapeGateOpen()
{
    UE_LOG(LogTemp, Warning, TEXT("Escape Gate:  %s"), bEscapeGateOpen ? TEXT("OPEN") : TEXT("CLOSED"));
}

void AT2PlayGameState::OnRep_SurvivorsAlive()
{
    UE_LOG(LogTemp, Warning, TEXT("Survivors Alive: %d"), AliveSurvivorCount);
}

void AT2PlayGameState::OnRep_MatchResult()
{
    UE_LOG(LogTemp, Warning, TEXT("Match Result Changed: %d"), (int32)MatchResult);
    // 여기서 결과 UI 표시 트리거
}