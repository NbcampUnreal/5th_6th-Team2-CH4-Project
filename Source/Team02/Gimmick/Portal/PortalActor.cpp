#include "Gimmick/Portal/PortalActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Character/PlayerCharacter/T2PlayerCharacter.h"
#include "PlayerState/Player/SurvivorPlayerState.h"
#include "GameState/T2GameStateBase.h"
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

	PortalEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("PortalEffect"));
	PortalEffect->SetupAttachment(RootComponent);
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

	GetWorldTimerManager().SetTimer(PortalTimerHandle, this, &APortalActor::UpdateTimer, 1.0f, true);

	UE_LOG(LogTemp, Warning, TEXT("Portal Activated! Time Limit: %.0f seconds"), PortalTimeLimit);
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

	UE_LOG(LogTemp, Warning, TEXT("Player %s entered portal! (%d players entered)"), 
		*SurvivorPS->GetPlayerName(), EnteredPlayers.Num());

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

		UE_LOG(LogTemp, Warning, TEXT("Player %s switched to spectator mode"), *SurvivorPS->GetPlayerName());
	}

	if (AreAllSurvivorsEntered())
	{
		UE_LOG(LogTemp, Warning, TEXT("All survivors entered! Transitioning to next map..."));
		GetWorldTimerManager().ClearTimer(PortalTimerHandle);
		TransitionToNextMap();
	}
}

void APortalActor::UpdateTimer()
{
	if (!HasAuthority()) return;

	RemainingTime -= 1.0f;

	UE_LOG(LogTemp, Log, TEXT("Portal Time Remaining: %.0f seconds (%d/%d players entered)"),
		RemainingTime, EnteredPlayers.Num(), GetWorld()->GetGameState<AT2GameStateBase>()->GetAliveSurvivorCount());

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

	KillRemainingPlayers();

	if (EnteredPlayers.Num() > 0)
	{
		TransitionToNextMap();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No players entered portal. Game Over."));
	}
}

bool APortalActor::AreAllSurvivorsEntered()
{
	if (!HasAuthority()) return false;

	AT2GameStateBase* GS = GetWorld()->GetGameState<AT2GameStateBase>();
	if (!GS) return false;

	int32 AliveSurvivors = GS->GetAliveSurvivorCount();
	
	UE_LOG(LogTemp, Log, TEXT("Alive Survivors: %d, Entered: %d"), 
		AliveSurvivors, EnteredPlayers.Num());

	return EnteredPlayers.Num() >= AliveSurvivors;
}

void APortalActor::KillRemainingPlayers()
{
	if (!HasAuthority()) return;

	TArray<AActor*> FoundCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AT2PlayerCharacter::StaticClass(), FoundCharacters);

	for (AActor* Actor : FoundCharacters)
	{
		AT2PlayerCharacter* Player = Cast<AT2PlayerCharacter>(Actor);
		if (!Player) continue;

		ASurvivorPlayerState* SurvivorPS = Cast<ASurvivorPlayerState>(Player->GetPlayerState());
		if (!SurvivorPS) continue;

		if (!SurvivorPS->bIsEscaped && !SurvivorPS->bIsDead)
		{
			UE_LOG(LogTemp, Warning, TEXT("Player %s failed to enter portal - Killed"), 
				*SurvivorPS->GetPlayerName());
			
			SurvivorPS->bIsDead = true;
			Player->OnDeath();
		}
	}
}

void APortalActor::TransitionToNextMap()
{
	if (!HasAuthority()) return;

	UE_LOG(LogTemp, Warning, TEXT("Transitioning to next map: %s"), *NextMapName.ToString());

	FTimerHandle TransitionTimer;
	GetWorldTimerManager().SetTimer(TransitionTimer, [this]()
	{
		GetWorld()->ServerTravel(FString::Printf(TEXT("/Game/Library_Pack/Maps/%s?listen"), *NextMapName.ToString()));
	}, 2.0f, false);
}