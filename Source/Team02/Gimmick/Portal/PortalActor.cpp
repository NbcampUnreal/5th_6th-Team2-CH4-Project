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
#include "T2PlayGameMod.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Controller/T2BaseController.h"

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
	
	if (AT2PlayGameState* GS = GetWorld()->GetGameState<AT2PlayGameState>())
	{
		GS->SetPortalActive(true);
		GS->SetPortalRemainingTime(RemainingTime);
	}
	
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
			UE_LOG(LogTemp, Warning, TEXT("Niagara Effect spawned successfully! (Role: %s)"),
				HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn Niagara Effect!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PortalNiagaraSystem is not set! Please assign it in Blueprint."));
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

	// Already escaped check
	if (SurvivorPS->bIsEscaped)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player %s already escaped"), *SurvivorPS->GetPlayerName());
		return;
	}

	if (EnteredPlayers.Contains(SurvivorPS))
	{
		UE_LOG(LogTemp, Warning, TEXT("Player %s already in EnteredPlayers list"), *SurvivorPS->GetPlayerName());
		return;
	}

	// Set escaped state (GameState count handled inside SetEscaped)
	SurvivorPS->SetEscaped();
	EnteredPlayers.Add(SurvivorPS);

	UE_LOG(LogTemp, Warning, TEXT("Player %s ESCAPED through portal! (%d players escaped)"), 
		*SurvivorPS->GetPlayerName(), EnteredPlayers.Num());

	// Show result UI (escaped) - Client RPC
	APlayerController* PC = Cast<APlayerController>(Player->GetController());
	if (AT2BaseController* T2PC = Cast<AT2BaseController>(PC))
	{
		T2PC->Client_ShowPersonalResult(true);
	}
	
	// Hide player and disable collision (only for the escaped player)
	Player->SetActorHiddenInGame(true);
	Player->SetActorEnableCollision(false);
	
	// DON'T call DisableInput here - it causes issues with other players
	// The result UI will handle input mode for the escaped player

	UE_LOG(LogTemp, Warning, TEXT("Player %s hidden"), *SurvivorPS->GetPlayerName());

	// Check win conditions
	if (AT2PlayGameMod* GM = GetWorld()->GetAuthGameMode<AT2PlayGameMod>())
	{
		GM->CheckWinConditions();
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

	// Update GameState with remaining time
	if (AT2PlayGameState* GS = GetWorld()->GetGameState<AT2PlayGameState>())
	{
		GS->SetPortalRemainingTime(RemainingTime);
	}

	if (RemainingTime <= 0.0f)
	{
		OnPortalTimeExpired();
	}
}

void APortalActor::OnPortalTimeExpired()
{
	if (!HasAuthority()) return;

	UE_LOG(LogTemp, Warning, TEXT("Portal time expired!"));
	GetWorldTimerManager().ClearTimer(PortalTimerHandle);
	
	bIsActive = false;

	// Notify GameState portal is closed
	if (AT2PlayGameState* GS = GetWorld()->GetGameState<AT2PlayGameState>())
	{
		GS->SetPortalActive(false);
	}

	UE_LOG(LogTemp, Warning, TEXT("Portal closed. %d player(s) escaped. Game continues."), EnteredPlayers.Num());
}

bool APortalActor::AreAllSurvivorsEntered()
{
	if (!HasAuthority()) return false;

	AT2PlayGameState* GS = GetWorld()->GetGameState<AT2PlayGameState>();
	if (!GS) return false;

	int32 AliveSurvivors = GS->GetAliveSurvivorCount();
	
	return AliveSurvivors <= 0;
}

void APortalActor::KillRemainingPlayers()
{
	// Not used
}

void APortalActor::TransitionToNextMap()
{
	// Not used - game end handled by T2PlayGameMod::EndMatch()
}
