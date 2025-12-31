// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TestGameMode.generated.h"

class AT2BaseController;
class APortalActor;

UCLASS()
class TEAM02_API ATestGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override; 
	virtual void Logout(AController* Exiting) override;
	
public:
	ATestGameMode();

	virtual APlayerController* SpawnPlayerController(ENetRole InRemoteRole, const FString& Options) override;
	
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Test Setup")
	TSubclassOf<APawn> TestSurvivorPawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Test Setup")
	TSubclassOf<APawn> TestKillerPawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Test Setup")
	TSubclassOf<APlayerState> TestSurvivorPlayerStateClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Test Setup")
	TSubclassOf<APlayerState> TestKillerPlayerStateClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Players")
	TArray<TObjectPtr<AT2BaseController>> AlivePlayerControllers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Players")
	TArray<TObjectPtr<AT2BaseController>> DeadPlayerControllers;


#pragma region Portal System
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick")
	TSubclassOf<APortalActor> PortalClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gimmick")
	TObjectPtr<APortalActor> CurrentPortal;

public:
	UFUNCTION(BlueprintCallable, Category = "Gimmick")
	void SpawnPortalAtRandomNavLocation();
#pragma endregion

protected:
	int32 PlayerCount = 0;
};