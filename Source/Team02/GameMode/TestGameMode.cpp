// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/TestGameMode.h"

#include "PlayerState/Player/SurvivorPlayerState.h"
#include "GameFramework/Controller.h"

ATestGameMode::ATestGameMode()
{
	PlayerControllerClass = APlayerController::StaticClass(); 
	
	PlayerStateClass = ASurvivorPlayerState::StaticClass();

	PlayerCount = 0;
}

APlayerController* ATestGameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	int32 CurrentNumPlayers = GetNumPlayers();

	TSubclassOf<APlayerState> OriginalPSClass = PlayerStateClass;
	TSubclassOf<APlayerController> OriginalPCClass = PlayerControllerClass;

	if (CurrentNumPlayers == 0) 
	{
		if (TestSurvivorControllerClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("Spawning Survivor Controller for Player 0"));
			PlayerControllerClass = TestSurvivorControllerClass;
		}
	}
	else if (CurrentNumPlayers == 1)
	{
		if (TestKillerControllerClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("Spawning Killer Controller for Player 1"));
			PlayerControllerClass = TestKillerControllerClass;

			if (TestKillerPlayerStateClass) 
			{
				UE_LOG(LogTemp, Warning, TEXT("Assigning KILLER PlayerState Class"));
				PlayerStateClass = TestKillerPlayerStateClass; 
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Current Num Players: %d"), CurrentNumPlayers);

	if (CurrentNumPlayers == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Assigning SURVIVOR Class"));
		PlayerControllerClass = TestSurvivorControllerClass;
	}
	else if (CurrentNumPlayers == 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("Assigning KILLER Class"));
		PlayerControllerClass = TestKillerControllerClass;
	}

	APlayerController* NewPC = Super::SpawnPlayerController(InRemoteRole, Options);
	
	PlayerControllerClass = OriginalPCClass;
	PlayerStateClass = OriginalPSClass;

	return NewPC;
}

UClass* ATestGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{

	if (InController && TestSurvivorControllerClass && InController->IsA(TestSurvivorControllerClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("Controller is Survivor Type. Assigning Survivor Pawn."));
		return TestSurvivorPawnClass;
	}

	if (InController && TestKillerControllerClass && InController->IsA(TestKillerControllerClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("Controller is Killer Type. Assigning Killer Pawn."));
		return TestKillerPawnClass;
	}

	UE_LOG(LogTemp, Warning, TEXT("Unknown Controller Type. Using Super implementation."));
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

void ATestGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	PlayerCount++;
}
