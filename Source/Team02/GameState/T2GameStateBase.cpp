#include "GameState/T2GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "PlayerState/Player/SurvivorPlayerState.h"

void AT2GameStateBase::AddKeyCount(int32 Amount)
{
	TotalKeyCount += Amount;
	OnRep_KeyCount();

	UE_LOG(LogTemp, Warning, TEXT("TotalKeyCount : %d"), TotalKeyCount);
}

void AT2GameStateBase::OnRep_KeyCount()
{
	//UI �ݿ�
}

void AT2GameStateBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AT2GameStateBase, TotalKeyCount);
}

int32 AT2GameStateBase::GetAliveSurvivorCount() const
{
	int32 AliveCount = 0;

	for (APlayerState* PS : PlayerArray)
	{
		ASurvivorPlayerState* SurvivorPS = Cast<ASurvivorPlayerState>(PS);
		if (SurvivorPS && !SurvivorPS->bIsDead && !SurvivorPS->bIsEscaped)
		{
			AliveCount++;
		}
	}

	return AliveCount;
}

int32 AT2GameStateBase::GetEscapedSurvivorCount() const
{
	int32 EscapedCount = 0;

	for (APlayerState* PS : PlayerArray)
	{
		ASurvivorPlayerState* SurvivorPS = Cast<ASurvivorPlayerState>(PS);
		if (SurvivorPS && SurvivorPS->bIsEscaped)
		{
			EscapedCount++;
		}
	}

	return EscapedCount;
}

void AT2GameStateBase::OnSurvivorEscaped()
{
	UE_LOG(LogTemp, Warning, TEXT("Survivor Escaped! Total Escaped: %d"), GetEscapedSurvivorCount());
	
}
