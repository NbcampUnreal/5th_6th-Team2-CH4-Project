// Fill out your copyright notice in the Description page of Project Settings.

#include "Gimmick/Portal/PortalActor.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameMode/TestGameMode.h"
#include "Controller/T2BaseController.h"
#include "PlayerState/T2PlayerState.h"
#include "Components/BillboardComponent.h"

APortalActor::APortalActor()
{
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	RootComponent = CollisionComponent;
	CollisionComponent->SetSphereRadius(100.f);
    
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &APortalActor::OnOverlapBegin);

	PortalEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PortalEffect"));
	PortalEffect->SetupAttachment(RootComponent);

	LightningEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LightningEffect"));
	LightningEffect->SetupAttachment(RootComponent);
	
	bReplicates = true;
	bAlwaysRelevant = true;

	bIsPortalActive = false;
	PortalTimeLimit = 300.0f; 
	RemainingTime = PortalTimeLimit;
	NextLevelName = FName("VictoryMap");

	PrimaryActorTick.bCanEverTick = false;
}

void APortalActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APortalActor, EnteredPlayers);
	DOREPLIFETIME(APortalActor, bIsPortalActive);
	DOREPLIFETIME(APortalActor, RemainingTime);
}

void APortalActor::BeginPlay()
{
	Super::BeginPlay();

	if (LightningEffect)
	{
		LightningEffect->SetRelativeLocation(FVector(0.f, 0.f, LightningHeight));
	}


	if (HasAuthority())
	{
		ActivatePortal();
	}
}

void APortalActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
								  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
								  bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !bIsPortalActive)
	{
		return;
	}

	APawn* OverlappingPawn = Cast<APawn>(OtherActor);
	if (!OverlappingPawn)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OverlappingPawn->GetController());
	if (!PC)
	{
		return;
	}

	AT2PlayerState* PS = PC->GetPlayerState<AT2PlayerState>();
	if (!PS)
	{
		return;
	}

	if (PS->PlayerRole == EPlayerRole::Killer)
	{
		UE_LOG(LogTemp, Warning, TEXT("Killer cannot enter the portal!"));
		return;
	}

	if (EnteredPlayers.Contains(PC))
	{
		return;
	}

	EnteredPlayers.Add(PC);

	ATestGameMode* GameMode = Cast<ATestGameMode>(UGameplayStatics::GetGameMode(this));
	
	UE_LOG(LogTemp, Warning, TEXT("Player %s entered the portal! (%d/%d)"), 
		*PC->GetName(), EnteredPlayers.Num(), GameMode ? GameMode->AlivePlayerControllers.Num() - 1 : 0);

	AT2BaseController* T2BC = Cast<AT2BaseController>(PC);
	if (T2BC)
	{
		APawn* ControlledPawn = T2BC->GetPawn();
		if (ControlledPawn)
		{
			ControlledPawn->SetActorHiddenInGame(true);
			ControlledPawn->SetActorEnableCollision(false);
			ControlledPawn->DisableInput(T2BC);
		}
		
	}

	Multicast_OnPlayerEntered(PC);

	if (CheckAllPlayersEntered())
	{
		UE_LOG(LogTemp, Warning, TEXT("All survivors entered! Transitioning to next level..."));
		TransitionToNextLevel();
	}
}

void APortalActor::ActivatePortal()
{
	if (!HasAuthority())
	{
		return;
	}

	bIsPortalActive = true;
	RemainingTime = PortalTimeLimit;

	UE_LOG(LogTemp, Warning, TEXT("Portal activated! Time limit: %.0f seconds"), PortalTimeLimit);

	GetWorldTimerManager().SetTimer(PortalTimerHandle, this, &APortalActor::OnPortalTimeout, PortalTimeLimit, false);

	GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &APortalActor::UpdateCountdown, 1.0f, true);

	Multicast_OnPortalActivated();
}

void APortalActor::UpdateCountdown()
{
	if (HasAuthority())
	{
		RemainingTime = FMath::Max(0.0f, RemainingTime - 1.0f);
	}
}

bool APortalActor::CheckAllPlayersEntered()
{
	if (!HasAuthority())
	{
		return false;
	}

	ATestGameMode* GameMode = Cast<ATestGameMode>(UGameplayStatics::GetGameMode(this));
	if (!GameMode)
	{
		return false;
	}

	int32 RequiredPlayers = 0;
	for (AT2BaseController* BC : GameMode->AlivePlayerControllers)
	{
		if (BC)
		{
			AT2PlayerState* PS = BC->GetPlayerState<AT2PlayerState>();
			if (PS && PS->PlayerRole == EPlayerRole::Survivor)
			{
				RequiredPlayers++;
			}
		}
	}

	return EnteredPlayers.Num() >= RequiredPlayers && RequiredPlayers > 0;
}

void APortalActor::OnPortalTimeout()
{
	if (!HasAuthority())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Portal timeout! Processing remaining players..."));

	ATestGameMode* GameMode = Cast<ATestGameMode>(UGameplayStatics::GetGameMode(this));
	if (!GameMode)
	{
		return;
	}

	TArray<AT2BaseController*> PlayersToKill;
	
	for (AT2BaseController* BC : GameMode->AlivePlayerControllers)
	{
		if (!BC)
		{
			continue;
		}

		AT2PlayerState* PS = BC->GetPlayerState<AT2PlayerState>();
		if (!PS)
		{
			continue;
		}

		if (PS->PlayerRole == EPlayerRole::Killer)
		{
			continue;
		}

		APlayerController* PC = Cast<APlayerController>(BC);
		if (PC && !EnteredPlayers.Contains(PC))
		{
			PlayersToKill.Add(BC);
		}
	}

	for (AT2BaseController* BC : PlayersToKill)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player %s did not enter portal in time. Marking as dead."), *BC->GetName());
		
		GameMode->AlivePlayerControllers.Remove(BC);
		GameMode->DeadPlayerControllers.Add(BC);
		
		if (APawn* Pawn = BC->GetPawn())
		{
			Pawn->SetActorHiddenInGame(true);
			Pawn->SetActorEnableCollision(false);
			Pawn->DisableInput(BC);
		}
	}

	TransitionToNextLevel();
}

void APortalActor::TransitionToNextLevel()
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(PortalTimerHandle);
	GetWorldTimerManager().ClearTimer(CountdownTimerHandle);

	bIsPortalActive = false;

	UGameplayStatics::OpenLevel(GetWorld(), NextLevelName);
}

bool APortalActor::HasPlayerEntered(APlayerController* PlayerController) const
{
	return EnteredPlayers.Contains(PlayerController);
}

void APortalActor::Multicast_OnPortalActivated_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Portal activated notification received!"));
	
	if (PortalEffect && !PortalEffect->IsActive())
	{
		PortalEffect->Activate(true);
	}

	if (LightningEffect)
	{
		APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
		if (LocalPC)
		{
			AT2PlayerState* PS = LocalPC->GetPlayerState<AT2PlayerState>();
			if (PS && PS->PlayerRole == EPlayerRole::Killer)
			{
				PortalEffect->SetVisibility(false);
				LightningEffect->SetVisibility(false);
			}
			else
			{
				PortalEffect->SetVisibility(true);
				LightningEffect->SetVisibility(true);
				
				if (!LightningEffect->IsActive())
				{
					LightningEffect->Activate(true);
				}
			}
		}
	}
	
}

void APortalActor::Multicast_OnPlayerEntered_Implementation(APlayerController* PlayerController)
{
	if (PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player entered portal notification: %s"), *PlayerController->GetName());
		
	}
}