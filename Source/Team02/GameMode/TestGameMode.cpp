// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/TestGameMode.h"

#include "PlayerState/Player/SurvivorPlayerState.h"
#include "GameFramework/Controller.h"
#include "Controller/Player/T2PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Gimmick/Portal/PortalActor.h"

void ATestGameMode::BeginPlay()
{
	Super::BeginPlay();

	SpawnPortalAtRandomNavLocation();
}

ATestGameMode::ATestGameMode()
{
	PlayerControllerClass = APlayerController::StaticClass(); 
	
	PlayerStateClass = ASurvivorPlayerState::StaticClass();

	PlayerCount = 0;
}

APlayerController* ATestGameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	int32 CurrentNumPlayers = GetNumPlayers();
	UE_LOG(LogTemp, Warning, TEXT("Current Num Players: %d"), CurrentNumPlayers);

	TSubclassOf<APlayerState> OriginalPSClass = PlayerStateClass;
	TSubclassOf<APlayerController> OriginalPCClass = PlayerControllerClass;

	if (CurrentNumPlayers == 0) 
	{
		if (TestSurvivorControllerClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("Assigning SURVIVOR Classes"));
			PlayerControllerClass = TestSurvivorControllerClass;
		}
	}
	else if (CurrentNumPlayers == 1)
	{
		if (TestKillerControllerClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("Assigning KILLER Classes"));
			PlayerControllerClass = TestKillerControllerClass;

			if (TestKillerPlayerStateClass) 
			{
				PlayerStateClass = TestKillerPlayerStateClass; 
			}
		}
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

	AT2PlayerController* NewPlayerController = Cast<AT2PlayerController>(NewPlayer);
	if (IsValid(NewPlayerController) == true)
	{
		AlivePlayerControllers.Add(NewPlayerController);
	}
}

void ATestGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	AT2PlayerController* ExitingPlayerController = Cast<AT2PlayerController>(Exiting);
	if (IsValid(ExitingPlayerController) == true && AlivePlayerControllers.Find(ExitingPlayerController) != INDEX_NONE)
	{
		AlivePlayerControllers.Remove(ExitingPlayerController);
		DeadPlayerControllers.Add(ExitingPlayerController);
	}
}

void ATestGameMode::OnCharacterDead(AT2PlayerController* InController)
{
	if (IsValid(InController) == false || AlivePlayerControllers.Find(InController) == INDEX_NONE)
	{
		return;
	}

	AlivePlayerControllers.Remove(InController);
	DeadPlayerControllers.Add(InController);

	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Alive Player Number : %d / Dead Player Number : %d"), AlivePlayerControllers.Num(), DeadPlayerControllers.Num()), true, true, FLinearColor::Green, 5.f);
}

void ATestGameMode::SpawnPortalAtRandomNavLocation()
{
	
	if (!PortalClass) 
	{
		UE_LOG(LogTemp, Error, TEXT("PortalClass is not set!"));
		return;
	}
	
	
	if (CurrentPortal)
	{
		CurrentPortal->Destroy();
		CurrentPortal = nullptr;
	}
	

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		UE_LOG(LogTemp, Error, TEXT("Navigation System not found!"));
		return;
	}

	FNavLocation RandomLocation;
	if (NavSys->GetRandomReachablePointInRadius(FVector::ZeroVector, 5000.f, RandomLocation))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		FVector SpawnPos = RandomLocation.Location + FVector(0.f, 0.f, 100.f);
		
		CurrentPortal = GetWorld()->SpawnActor<APortalActor>(PortalClass, SpawnPos, FRotator::ZeroRotator, SpawnParams);
		
		if (CurrentPortal)
		{
			UE_LOG(LogTemp, Warning, TEXT("Portal spawned at location: %s"), *SpawnPos.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn portal!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to find random navigation point!"));
	}
}

/*
void ATestGameMode::StartPortalTimer(float TimeLimit)
{
	if (CurrentPortal)
	{
		FTimerHandle PortalTimerHandle;
		GetWorldTimerManager().SetTimer(PortalTimerHandle, [this]()
		{
			if (CurrentPortal)
			{
				CurrentPortal->SetPortalActive(false);
				UE_LOG(LogTemp, Warning, TEXT("Portal deactivated due to time limit!"));
			}
		}, TimeLimit, false);
	}
}
*/