// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/TestGameMode.h"

#include "Controller/T2BaseController.h"
#include "PlayerState/T2PlayerState.h"
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
	PlayerControllerClass = AT2BaseController::StaticClass(); 
	PlayerCount = 0;
}

APlayerController* ATestGameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	int32 CurrentNumPlayers = GetNumPlayers();
    
	if (CurrentNumPlayers == 0) 
	{
		PlayerStateClass = TestKillerPlayerStateClass;
	}
	else
	{
		PlayerStateClass = TestSurvivorPlayerStateClass;
	}

	return Super::SpawnPlayerController(InRemoteRole, Options);
}

UClass* ATestGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (AT2PlayerState* PS = InController->GetPlayerState<AT2PlayerState>())
	{
		if (PS->PlayerRole == EPlayerRole::Survivor)
		{
			return TestSurvivorPawnClass;
		}
		else if (PS->PlayerRole == EPlayerRole::Killer)
		{
			return TestKillerPawnClass;
		}
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

void ATestGameMode::PostLogin(APlayerController* NewPlayer)
{
	if (AT2PlayerState* PS = NewPlayer->GetPlayerState<AT2PlayerState>())
	{
		int32 PlayerIdx = GetNumPlayers() - 1;
		EPlayerRole AssignedRole = (PlayerIdx == 0) ? EPlayerRole::Killer : EPlayerRole::Survivor;
        
		PS->PlayerRole = AssignedRole; 
        
		UE_LOG(LogTemp, Warning, TEXT("Player %d assigned role: %d"), PlayerIdx, (int32)AssignedRole);
	}

	Super::PostLogin(NewPlayer);
    
	if (AT2BaseController* BC = Cast<AT2BaseController>(NewPlayer))
	{
		AlivePlayerControllers.Add(BC);
        
		if (BC->IsLocalPlayerController())
		{
			BC->UpdateHUDForRole(BC->GetPlayerState<AT2PlayerState>()->PlayerRole);
		}
	}
    
	PlayerCount++;
}

void ATestGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	AT2BaseController* ExitingPlayerController = Cast<AT2BaseController>(Exiting);
	if (IsValid(ExitingPlayerController) == true && AlivePlayerControllers.Find(ExitingPlayerController) != INDEX_NONE)
	{
		AlivePlayerControllers.Remove(ExitingPlayerController);
		DeadPlayerControllers.Add(ExitingPlayerController);
	}
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