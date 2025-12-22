// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TestGameMode.generated.h"

class AT2PlayerController;

UCLASS()
class TEAM02_API ATestGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
public:
	ATestGameMode();

	virtual APlayerController* SpawnPlayerController(ENetRole InRemoteRole, const FString& Options) override;
	
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	void OnCharacterDead(AT2PlayerController* InController);

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<AT2PlayerController>> AlivePlayerControllers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<AT2PlayerController>> DeadPlayerControllers;


protected:
	int32 PlayerCount;

	virtual void PostLogin(APlayerController* NewPlayer) override; 

	virtual void Logout(AController* Exiting) override;

#pragma region Potal System
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick")
	TSubclassOf<class APortalActor> PortalClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gimmick")
	TObjectPtr<class APortalActor> CurrentPortal;
	

public:
	UFUNCTION(BlueprintCallable, Category = "Gimmick")
	void SpawnPortalAtRandomNavLocation();
	
	//void StartPortalTimer(float TimeLimit);

#pragma endregion
};