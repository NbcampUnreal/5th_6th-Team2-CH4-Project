#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "T2GameInstance.h"
#include "TimerManager.h"
#include "T2PlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerRoleChanged, EPlayerRole, NewRole);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnVisionDebuffChanged, bool);

UCLASS()
class TEAM02_API AT2PlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    AT2PlayerState();

    UPROPERTY(ReplicatedUsing = OnRep_PlayerRole, BlueprintReadOnly, Category = "Role")
    EPlayerRole PlayerRole = EPlayerRole::None;


    UPROPERTY(BlueprintAssignable, Category = "Role")
    FOnPlayerRoleChanged OnPlayerRoleChanged;


    void SetPlayerRole(EPlayerRole NewRole);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    UFUNCTION()
    void OnRep_PlayerRole();
};