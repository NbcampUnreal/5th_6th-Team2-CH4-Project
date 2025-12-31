#include "PlayerState/T2PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Gimmick/Player/ItemBase.h"
#include "EngineUtils.h"

AT2PlayerState::AT2PlayerState()
{
    bReplicates = true;
}

void AT2PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AT2PlayerState, PlayerRole);
}

void AT2PlayerState::SetPlayerRole(EPlayerRole NewRole)
{
    if (HasAuthority())
    {
        PlayerRole = NewRole;
        OnPlayerRoleChanged.Broadcast(PlayerRole);
    }
}

void AT2PlayerState::OnRep_PlayerRole()
{
    OnPlayerRoleChanged.Broadcast(PlayerRole);

    AController* OwnerController = Cast<AController>(GetOwner());
    if (!OwnerController) return;

    if (!OwnerController->IsLocalController()) return;

    if (PlayerRole == EPlayerRole::Killer)
    {
        for (TActorIterator<AItemBase> It(GetWorld()); It; ++It)
        {
            It->HideForLocalPlayer();
            
        }
    }

}