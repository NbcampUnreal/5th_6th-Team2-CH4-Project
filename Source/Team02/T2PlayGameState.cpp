#include "T2PlayGameState.h"

#include "T2PlayGameMod.h"
#include "Net/UnrealNetwork.h"

AT2PlayGameState::AT2PlayGameState()
{
}

void AT2PlayGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AT2PlayGameState, TotalKeyCount);
    DOREPLIFETIME(AT2PlayGameState, RequiredKeys);
    DOREPLIFETIME(AT2PlayGameState, bEscapeGateOpen);
    DOREPLIFETIME(AT2PlayGameState, TotalSurvivors);
    DOREPLIFETIME(AT2PlayGameState, SurvivorsAlive);
    DOREPLIFETIME(AT2PlayGameState, SurvivorsEscaped);
    DOREPLIFETIME(AT2PlayGameState, MatchResult);
    DOREPLIFETIME(AT2PlayGameState, bIsPortalActive);
    DOREPLIFETIME(AT2PlayGameState, PortalRemainingTime);
}

void AT2PlayGameState::AddKeyCount(int32 Amount)
{
    if (GetLocalRole() != ROLE_Authority) return;

    TotalKeyCount += Amount;

    UE_LOG(LogTemp, Warning, TEXT("Key Collected!  Total:  %d / %d"), TotalKeyCount, RequiredKeys);

    if (AT2PlayGameMod* GM = Cast<AT2PlayGameMod>(GetWorld()->GetAuthGameMode()))
    {
        GM->OnKeyCollected(TotalKeyCount);
    }
    
    if (TotalKeyCount >= RequiredKeys)
    {
        SetEscapeGateOpen(true);
    }
}

void AT2PlayGameState::OnRep_KeyCount()
{
    UE_LOG(LogTemp, Warning, TEXT("Keys: %d / %d (Gate)"), TotalKeyCount, RequiredKeys);
}

void AT2PlayGameState::AddCollectedKey()
{
    AddKeyCount(1);
}

void AT2PlayGameState::OnSurvivorDied()
{
    if (GetLocalRole() != ROLE_Authority) return;

    SurvivorsAlive = FMath::Max(0, SurvivorsAlive - 1);
}

void AT2PlayGameState::OnSurvivorEscaped()
{
    if (GetLocalRole() != ROLE_Authority) return;

    SurvivorsEscaped++;
    SurvivorsAlive = FMath::Max(0, SurvivorsAlive - 1);
}

void AT2PlayGameState::SetEscapeGateOpen(bool bOpen)
{
    if (GetLocalRole() != ROLE_Authority) return;

    bEscapeGateOpen = bOpen;
    
    if (bOpen)
    {
        UE_LOG(LogTemp, Warning, TEXT("Escape Gate OPENED!"));
    }
}

void AT2PlayGameState::SetMatchResult(EMatchResult Result)
{
    if (GetLocalRole() != ROLE_Authority) return;

    MatchResult = Result;
}

void AT2PlayGameState::SetPortalActive(bool bActive)
{
    if (GetLocalRole() != ROLE_Authority) return;

    bIsPortalActive = bActive;
    UE_LOG(LogTemp, Warning, TEXT("Portal Active: %s"), bActive ? TEXT("TRUE") : TEXT("FALSE"));
}

void AT2PlayGameState::SetPortalRemainingTime(float Time)
{
    if (GetLocalRole() != ROLE_Authority) return;

    PortalRemainingTime = Time;
}

void AT2PlayGameState::OnRep_PortalActive()
{
    UE_LOG(LogTemp, Warning, TEXT("OnRep_PortalActive: %s"), bIsPortalActive ? TEXT("ACTIVE") : TEXT("INACTIVE"));
}

void AT2PlayGameState::OnRep_PortalRemainingTime()
{
    // UI에서 바인딩으로 자동 업데이트됨
}

void AT2PlayGameState::OnRep_CollectedKeys()
{
    UE_LOG(LogTemp, Warning, TEXT("Keys Collected:  %d / %d"), TotalKeyCount, RequiredKeys);
}

void AT2PlayGameState::OnRep_EscapeGateOpen()
{
    UE_LOG(LogTemp, Warning, TEXT("Escape Gate:  %s"), bEscapeGateOpen ? TEXT("OPEN") : TEXT("CLOSED"));
}

void AT2PlayGameState::OnRep_SurvivorsAlive()
{
    UE_LOG(LogTemp, Warning, TEXT("Survivors Alive: %d"), SurvivorsAlive);
}

void AT2PlayGameState::OnRep_MatchResult()
{
    UE_LOG(LogTemp, Warning, TEXT("Match Result Changed: %d"), (int32)MatchResult);
}
