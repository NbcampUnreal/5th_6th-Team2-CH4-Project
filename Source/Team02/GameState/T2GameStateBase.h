#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "T2GameStateBase.generated.h"

UCLASS()
class TEAM02_API AT2GameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(ReplicatedUsing = OnRep_KeyCount)
	int32 TotalKeyCount;

	void AddKeyCount(int32 Amount);

	UFUNCTION()
	void OnRep_KeyCount();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Game State")
	int32 GetKeyCount() const { return TotalKeyCount; }

	UFUNCTION(BlueprintCallable, Category = "Game State")
	int32 GetAliveSurvivorCount() const;

	UFUNCTION(BlueprintCallable, Category = "Game State")
	int32 GetEscapedSurvivorCount() const;

	void OnSurvivorEscaped();
};
