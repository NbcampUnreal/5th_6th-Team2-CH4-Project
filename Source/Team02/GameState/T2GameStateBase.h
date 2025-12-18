#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "T2GameStateBase.generated.h"

UCLASS()
class TEAM02_API AT2GameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 Countdown = -1;  //UI version

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;







//public:
//	UPROPERTY(ReplicatedUsing = OnRep_KeyCount)
//	int32 TotalKeyCount;
//
//	void AddKeyCount(int32 Amount);
//
//	UFUNCTION()
//	void OnRep_KeyCount();
//
//	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
