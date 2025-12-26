#include "GameState/T2GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "PlayerState/Player/SurvivorPlayerState.h"

void AT2GameStateBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AT2GameStateBase, RequiredPlayers);
}
