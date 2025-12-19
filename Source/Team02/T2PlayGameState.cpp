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
    DOREPLIFETIME(AT2PlayGameState, TotalSurvivors);
    DOREPLIFETIME(AT2PlayGameState, SurvivorsAlive);
    DOREPLIFETIME(AT2PlayGameState, SurvivorsEscaped);
    DOREPLIFETIME(AT2PlayGameState, MatchResult);
}

// �� ���� �Լ����� ����! ��
void AT2PlayGameState::AddKeyCount(int32 Amount)
{
    if (GetLocalRole() != ROLE_Authority) return;

    CollectedKeys += Amount;

    UE_LOG(LogTemp, Warning, TEXT("Key Collected!  Total:  %d / %d"), CollectedKeys, RequiredKeys);

    // ���� �� ������ Ż�⹮ ����
    if (CollectedKeys >= RequiredKeys)
    {
        SetEscapeGateOpen(true);
    }
}

// ���� �Լ� (AddKeyCount ȣ��)
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
}

void AT2PlayGameState::SetMatchResult(EMatchResult Result)
{
    if (GetLocalRole() != ROLE_Authority) return;

    MatchResult = Result;
}

void AT2PlayGameState::OnRep_CollectedKeys()
{
    UE_LOG(LogTemp, Warning, TEXT("Keys Collected:  %d / %d"), CollectedKeys, RequiredKeys);
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