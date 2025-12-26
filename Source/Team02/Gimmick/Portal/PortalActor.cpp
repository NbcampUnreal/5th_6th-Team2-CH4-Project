#include "Gimmick/Portal/PortalActor.h"


#include "EngineUtils.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Character/PlayerCharacter/T2PlayerCharacter.h"
#include "Components/PointLightComponent.h"
#include "PlayerState/Player/SurvivorPlayerState.h"
#include "T2PlayGameState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpectatorPawn.h"

APortalActor::APortalActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
	RootComponent = PortalMesh;
	PortalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TriggerVolume = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(RootComponent);
	TriggerVolume->SetSphereRadius(150.0f);
	TriggerVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	PortalEffect = nullptr;
	
	PortalLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PortalLight"));
	PortalLight->SetupAttachment(RootComponent);
	PortalLight->SetIntensity(10000.0f); 
	PortalLight->SetAttenuationRadius(5000.0f); 
	PortalLight->SetLightColor(FLinearColor(0.0f, 1.0f, 1.0f));  
	PortalLight->SetVisibility(false); 
	PortalLight->CastShadows = false;

	PortalTimeLimit = 120.0f;
}

void APortalActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &APortalActor::OnOverlapBegin);
		RemainingTime = PortalTimeLimit;
	}
}

void APortalActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APortalActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APortalActor, EnteredPlayers);
	DOREPLIFETIME(APortalActor, bIsActive);
	DOREPLIFETIME(APortalActor, RemainingTime);
}

void APortalActor::ActivatePortal()
{
	if (!HasAuthority()) return;

	bIsActive = true;
	RemainingTime = PortalTimeLimit;
	
	SpawnPortalEffects();

	GetWorldTimerManager().SetTimer(PortalTimerHandle, this, &APortalActor::UpdateTimer, 1.0f, true);


	
	UE_LOG(LogTemp, Warning, TEXT("Portal Activated! Time Limit: %.0f seconds"), PortalTimeLimit);
}

void APortalActor::OnRep_IsActive()
{
	if (bIsActive)
	{
		SpawnPortalEffects();
	}
}

void APortalActor::SpawnPortalEffects()
{
	if (PortalNiagaraSystem)
	{
		if (PortalEffect)
		{
			PortalEffect->DestroyComponent();
			PortalEffect = nullptr;
		}

		PortalEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
			PortalNiagaraSystem,
			RootComponent,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			false 
		);
		
		if (PortalEffect)
		{
			UE_LOG(LogTemp, Warning, TEXT(" Niagara Effect spawned successfully! (Role: %s)"),
				HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT(" Failed to spawn Niagara Effect!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("️ PortalNiagaraSystem is not set! Please assign it in Blueprint."));
	}
	
	if (PortalLight)
	{
		PortalLight->SetVisibility(true);
	}

#if !UE_BUILD_SHIPPING
	if (bShowDebugSphere && HasAuthority())
	{
		FVector SphereLocation = GetActorLocation() + FVector(0, 0, 2000); 
		
		DrawDebugSphere(
			GetWorld(),
			SphereLocation,
			800.0f,  
			32,
			FColor::Cyan,
			false,
			PortalTimeLimit + 10.0f,  
			0,
			15.0f  
		);

		DrawDebugString(
			GetWorld(),
			SphereLocation + FVector(0, 0, 1000),
			TEXT("PORTAL HERE!"),
			nullptr,
			FColor::White,
			PortalTimeLimit + 10.0f,
			true, 
			3.0f  
		);
	}
#endif
}


void APortalActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                  bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !bIsActive) return;

	AT2PlayerCharacter* Player = Cast<AT2PlayerCharacter>(OtherActor);
	if (!Player) return;

	ASurvivorPlayerState* SurvivorPS = Cast<ASurvivorPlayerState>(Player->GetPlayerState());
	if (!SurvivorPS || SurvivorPS->bIsDead || SurvivorPS->bIsEscaped) return;

	OnPlayerEnterPortal(Player);
}

void APortalActor::OnPlayerEnterPortal(AT2PlayerCharacter* Player)
{
	if (!HasAuthority() || !Player) return;

	ASurvivorPlayerState* SurvivorPS = Cast<ASurvivorPlayerState>(Player->GetPlayerState());
	if (!SurvivorPS) return;

	if (EnteredPlayers.Contains(SurvivorPS))
	{
		UE_LOG(LogTemp, Warning, TEXT("Player %s already entered portal"), *SurvivorPS->GetPlayerName());
		return;
	}

	SurvivorPS->SetEscaped();
	EnteredPlayers.Add(SurvivorPS);

	UE_LOG(LogTemp, Warning, TEXT("Player %s ESCAPED through portal! (%d players escaped) - VICTORY!"), 
		*SurvivorPS->GetPlayerName(), EnteredPlayers.Num());

	if (AT2PlayGameState* GS = GetWorld()->GetGameState<AT2PlayGameState>())
	{
		GS->OnSurvivorEscaped();
	}
	
	APlayerController* PC = Cast<APlayerController>(Player->GetController());
	if (PC)
	{
		Player->SetActorHiddenInGame(true);
		Player->SetActorEnableCollision(false);

		PC->UnPossess();
		PC->ChangeState(NAME_Spectating);
		
		FVector SpectatorLocation = GetActorLocation() + FVector(0, 0, 500);
		PC->SetControlRotation(FRotator(-45, 0, 0));
		
		if (ASpectatorPawn* SpectatorPawn = PC->GetSpectatorPawn())
		{
			SpectatorPawn->SetActorLocation(SpectatorLocation);
		}

		UE_LOG(LogTemp, Warning, TEXT("Player %s switched to spectator mode in main map"), *SurvivorPS->GetPlayerName());
	}

	if (AreAllSurvivorsEntered())
	{
		UE_LOG(LogTemp, Warning, TEXT("All alive survivors entered portal! Transitioning to next map..."));
		GetWorldTimerManager().ClearTimer(PortalTimerHandle);
		TransitionToNextMap();
	}
}

APortalActor* APortalActor::FindActivePortal(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;
	
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;
	
	for (TActorIterator<APortalActor> It(World); It; ++It)
	{
		if (It->IsPortalActive())
		{
			return *It;
		}
	}
	
	return nullptr;
}

void APortalActor::UpdateTimer()
{
	if (!HasAuthority()) return;

	RemainingTime -= 1.0f;

	AT2PlayGameState* GS = GetWorld()->GetGameState<AT2PlayGameState>();
	if (GS)
	{
		UE_LOG(LogTemp, Log, TEXT("Portal Time Remaining: %.0f seconds (%d/%d players escaped)"),
			RemainingTime, EnteredPlayers.Num(), GS->GetAliveSurvivorCount());
	}

	// 제한시간 종료
	if (RemainingTime <= 0.0f)
	{
		OnPortalTimeExpired();
	}
}

void APortalActor::OnPortalTimeExpired()
{
	if (!HasAuthority()) return;

	UE_LOG(LogTemp, Warning, TEXT("Portal time expired! (2 minutes passed)"));
	GetWorldTimerManager().ClearTimer(PortalTimerHandle);

	KillRemainingPlayers();

	if (EnteredPlayers.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%d player(s) escaped successfully! Transitioning to next map..."), 
			EnteredPlayers.Num());
		TransitionToNextMap();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No players escaped portal. All players failed."));
	}
}

bool APortalActor::AreAllSurvivorsEntered()
{
	if (!HasAuthority()) return false;

	AT2PlayGameState* GS = GetWorld()->GetGameState<AT2PlayGameState>();
	if (!GS) return false;

	int32 AliveSurvivors = GS->GetAliveSurvivorCount();
	
	UE_LOG(LogTemp, Log, TEXT("Checking completion - Alive Survivors: %d, Entered: %d"), 
		AliveSurvivors, EnteredPlayers.Num());

	return EnteredPlayers.Num() >= AliveSurvivors;
}

void APortalActor::KillRemainingPlayers()
{
	if (!HasAuthority()) return;

	TArray<AActor*> FoundCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AT2PlayerCharacter::StaticClass(), FoundCharacters);

	int32 KilledCount = 0;
	for (AActor* Actor : FoundCharacters)
	{
		AT2PlayerCharacter* Player = Cast<AT2PlayerCharacter>(Actor);
		if (!Player) continue;

		ASurvivorPlayerState* SurvivorPS = Cast<ASurvivorPlayerState>(Player->GetPlayerState());
		if (!SurvivorPS) continue;

		if (!SurvivorPS->bIsEscaped && !SurvivorPS->bIsDead)
		{
			UE_LOG(LogTemp, Warning, TEXT("Player %s failed to enter portal within time limit - KILLED"), 
				*SurvivorPS->GetPlayerName());
			
			SurvivorPS->bIsDead = true;
			Player->OnDeath();
			KilledCount++;

			if (AT2PlayGameState* GS = GetWorld()->GetGameState<AT2PlayGameState>())
			{
				GS->OnSurvivorDied();
			}
		}
	}
	
	if (KilledCount > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Portal timeout: %d player(s) killed for not entering in time"), KilledCount);
	}
}

void APortalActor::TransitionToNextMap()
{
	if (!HasAuthority()) return;

	UE_LOG(LogTemp, Warning, TEXT("Transitioning to next map: %s"), *NextMapName.ToString());
	UE_LOG(LogTemp, Warning, TEXT("Escaped Players: %d"), EnteredPlayers.Num());

	FTimerHandle TransitionTimer;
	GetWorldTimerManager().SetTimer(TransitionTimer, [this]()
	{
		GetWorld()->ServerTravel(FString::Printf(TEXT("/Game/Library_Pack/Maps/%s?listen"), *NextMapName.ToString()));
	}, 2.0f, false);
}