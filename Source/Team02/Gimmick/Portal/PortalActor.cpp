// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/Portal/PortalActor.h"

#include "PortalActor.h"

#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

APortalActor::APortalActor()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(true);
	
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	RootComponent = CollisionComponent;
	CollisionComponent->SetSphereRadius(100.f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
    
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &APortalActor::OnOverlapBegin);

	PortalEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PortalEffect"));
	PortalEffect->SetupAttachment(RootComponent);;
	bIsActive = true;
}

void APortalActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APortalActor, EscapedPlayers);
	DOREPLIFETIME(APortalActor, bIsActive);
}

void APortalActor::BeginPlay()
{
	Super::BeginPlay();

	if (!bIsActive && PortalEffect)
	{
		PortalEffect->Deactivate();
	}
}

void APortalActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                  bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!bIsActive)
	{
		return;
	}

	if (!OtherActor || !OtherActor->IsA(APawn::StaticClass()))
	{
		return;
	}

	if (EscapedPlayers.Contains(OtherActor))
	{
		return;
	}

	if (!CanActorEscape(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Killer cannot escape through portal!"));
		return;
	}

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC)
	{
		return;
	}

	EscapedPlayers.Add(OtherActor);

	UE_LOG(LogTemp, Warning, TEXT("Player %s escaped through portal!"), *OtherActor->GetName());

	UGameplayStatics::OpenLevel(PC, "VictoryMap");
}

void APortalActor::OnRep_IsActive()
{
	if (PortalEffect)
	{
		if (bIsActive)
		{
			PortalEffect->Activate();
		}
		else
		{
			PortalEffect->Deactivate();
		}
	}

	if (CollisionComponent)
	{
		if (bIsActive)
		{
			CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		else
		{
			CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void APortalActor::SetPortalActive(bool bActive)
{
	if (!HasAuthority())
	{
		return;
	}

	bIsActive = bActive;
	OnRep_IsActive();
}

bool APortalActor::CanActorEscape(AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	APawn* Pawn = Cast<APawn>(Actor);
	if (!Pawn)
	{
		return false;
	}

	AController* Controller = Pawn->GetController();
	if (!Controller)
	{
		return false;
	}

	FString ControllerClassName = Controller->GetClass()->GetName();
	if (ControllerClassName.Contains(TEXT("KillerController")))
	{
		return false;
	}
	
	return true;
}
