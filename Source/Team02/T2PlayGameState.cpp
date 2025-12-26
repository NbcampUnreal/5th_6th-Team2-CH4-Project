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
}

// �� ���� �Լ����� ����! ��
void AT2PlayGameState::AddKeyCount(int32 Amount)
{
    if (GetLocalRole() != ROLE_Authority) return;

    TotalKeyCount += Amount;

    UE_LOG(LogTemp, Warning, TEXT("Key Collected!  Total:  %d / %d"), TotalKeyCount, RequiredKeys);

    if (AT2PlayGameMod* GM = Cast<AT2PlayGameMod>(GetWorld()->GetAuthGameMode()))
    {
        GM->OnKeyCollected(TotalKeyCount);
    }
    
    // ���� �� ������ Ż�⹮ ����
    if (TotalKeyCount >= RequiredKeys)
    {
        SetEscapeGateOpen(true);
    }
}

void AT2PlayGameState::OnRep_KeyCount()
{
    UE_LOG(LogTemp, Warning, TEXT("Keys: %d / %d (Gate)"), TotalKeyCount, RequiredKeys);
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