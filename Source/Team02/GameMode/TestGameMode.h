// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TestGameMode.generated.h"

/**
 * 
 */
UCLASS()
class TEAM02_API ATestGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ATestGameMode();

	virtual APlayerController* SpawnPlayerController(ENetRole InRemoteRole, const FString& Options) override;
	
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Test Setup")
	TSubclassOf<APawn> TestSurvivorPawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Test Setup")
	TSubclassOf<APawn> TestKillerPawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Test Setup")
	TSubclassOf<APlayerController> TestSurvivorControllerClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Test Setup")
	TSubclassOf<APlayerController> TestKillerControllerClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Test Setup")
	TSubclassOf<APlayerState> TestKillerPlayerStateClass; 


protected:
	int32 PlayerCount;

	virtual void PostLogin(APlayerController* NewPlayer) override; 
};