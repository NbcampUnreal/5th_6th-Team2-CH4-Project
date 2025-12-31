#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "T2GameStateBase.generated.h"


UCLASS()
class TEAM02_API AT2GameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	int32 GetCurrentPlayerCount() const { return PlayerArray.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	int32 GetRequiredPlayerCount() const { return RequiredPlayers; }

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Lobby")
	int32 RequiredPlayers = 3;
};