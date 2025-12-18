#include "PlayerState/T2PlayerState.h"
#include "Net/UnrealNetwork.h"

AT2PlayerState::AT2PlayerState()
{
}

void AT2PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AT2PlayerState, PlayerRole);
}

void AT2PlayerState::SetPlayerRole(EPlayerRole NewRole)
{
    if (!HasAuthority()) return;
    
    PlayerRole = NewRole;
    OnRep_PlayerRole();
    //OnPlayerRoleChanged.Broadcast(PlayerRole);
    
}

void AT2PlayerState::OnRep_PlayerRole()
{
   // OnPlayerRoleChanged.Broadcast(PlayerRole);
}