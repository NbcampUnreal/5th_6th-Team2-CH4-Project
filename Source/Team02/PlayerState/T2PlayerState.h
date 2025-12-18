#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "T2GameInstance.h"
#include "T2PlayerState.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerRoleChanged, EPlayerRole, NewRole);

UENUM(BlueprintType)
enum class EPlayerRole : uint8
{
    None,
    Killer,
    Survivor
};

UCLASS()
class TEAM02_API AT2PlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    AT2PlayerState();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


    UPROPERTY(ReplicatedUsing = OnRep_PlayerRole, BlueprintReadOnly, Category = "Role")
    EPlayerRole PlayerRole = EPlayerRole::None;

    UPROPERTY(Replicated, BlueprintReadOnly)
    bool bIsAlive = true;

  /*  UPROPERTY(BlueprintAssignable, Category = "Role")
    FOnPlayerRoleChanged OnPlayerRoleChanged;*/

    void SetPlayerRole(EPlayerRole NewRole);

    UFUNCTION()
    void OnRep_PlayerRole();
};