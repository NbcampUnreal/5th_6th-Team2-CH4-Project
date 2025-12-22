#include "Gimmick/Player/ItemBase.h"
#include "Components/CapsuleComponent.h"
#include "Character/PlayerCharacter/T2PlayerCharacter.h"
#include "Controller/Player/T2PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "Components/BoxComponent.h"

AItemBase::AItemBase()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;

	InteractiongBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractiongBox"));
	RootComponent = InteractiongBox;

	InteractiongBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

void AItemBase::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(InteractiongBox) == true)
	{
		InteractiongBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnOverlapBegin);
		InteractiongBox->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnOverlapEnd);
	}
	
}

void AItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AItemBase, InteractingPC);
}

void AItemBase::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (AT2PlayerCharacter* Player = Cast<AT2PlayerCharacter>(OtherActor))
	{
		Player->SetInteractableItem(this);
	}

	UE_LOG(LogTemp, Warning, TEXT("OverlapBegin"));
	DebugDrawCapsule();
}

void AItemBase::OnOverlapEnd(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (AT2PlayerCharacter* Player = Cast<AT2PlayerCharacter>(OtherActor))
	{
		if (AT2PlayerController* PC = Cast<AT2PlayerController>(Player->GetController()))
		{
			Player->ClearInteractableItem(this);
			PC->StopInteractUI();
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("OverlapEnd"));
}

void AItemBase::BeginInteract(AT2PlayerCharacter* Player)
{
	if (!HasAuthority()) return;

	if (InteractingPC != nullptr) return;

	AT2PlayerController* PC = Cast<AT2PlayerController>(Player->GetController());
	if (IsValid(PC) == true)
	{
		InteractingPC = PC;
	}
}

void AItemBase::CompleteInteract(AT2PlayerCharacter* Player)
{
	if (!HasAuthority()) return;

	if (Player->GetController() != InteractingPC) return;

	//Player->AddItemToInventory(this);   Inventory Item Add  (Comming Soon)
	Destroy();

}

void AItemBase::CanelInteract(AT2PlayerCharacter* Player)
{
	if (!HasAuthority()) return;
	
	if (InteractingPC == Player->GetController())
	{
		InteractingPC = nullptr;
	}
}


void AItemBase::OnRep_InteractingPC()
{
	//UpdateInteractionState();  UI "Other Player Using"
}

void AItemBase::DebugDrawCapsule()
{
	if (!InteractiongBox) return;

	const FVector Location = InteractiongBox->GetComponentLocation();
	const FQuat Rotation = InteractiongBox->GetComponentQuat();
	const FVector Extent = InteractiongBox->GetScaledBoxExtent(); // ÇÙ½É

	DrawDebugBox(
		GetWorld(),
		Location,
		Extent,
		Rotation,
		FColor::Green,
		false,
		0.1f,
		0,
		2.f
	);
}


