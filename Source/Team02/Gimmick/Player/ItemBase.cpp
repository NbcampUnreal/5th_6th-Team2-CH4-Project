#include "Gimmick/Player/ItemBase.h"
#include "Components/CapsuleComponent.h"
#include "Character/PlayerCharacter/T2PlayerCharacter.h"
#include "Controller/T2BaseController.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "Components/BoxComponent.h"
#include "Character/KillerCharacter/T2KillerCharacter.h"
#include "PlayerState/Player/SurvivorPlayerState.h"
#include "PlayerState/Player/InventoryComponent.h"


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
	MeshComponent->SetRenderCustomDepth(true);
	MeshComponent->CustomDepthStencilValue = 1;
} 

void AItemBase::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(InteractiongBox) == true)
	{
		InteractiongBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnOverlapBegin);
		InteractiongBox->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnOverlapEnd);
	}
	SetOutlineEnabled(false);
}

void AItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AItemBase, InteractingPC);
	DOREPLIFETIME(AItemBase, ItemID);
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
		Player->AddNearbyItem(this);
	}
	//DebugDrawCapsule();
}

void AItemBase::OnOverlapEnd(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (AT2PlayerCharacter* Player = Cast<AT2PlayerCharacter>(OtherActor))
	{
		if (AT2BaseController* PC = Cast<AT2BaseController>(Player->GetController()))
		{
			Player->RemoveNearbyItem(this);
			PC->StopInteractUI();
		}
	}
}

void AItemBase::BeginInteract(AT2PlayerCharacter* Player)
{
	if (!HasAuthority()) return;

	if (InteractingPC != nullptr) return;

	AT2BaseController* PC = Cast<AT2BaseController>(Player->GetController());
	if (IsValid(PC) == true)
	{
		InteractingPC = PC;
	}
}

void AItemBase::CompleteInteract(AT2PlayerCharacter* Player)
{
	if (!HasAuthority()) return;

	if (Player->GetController() != InteractingPC) return;

	if (auto* PS = Player->GetPlayerState<ASurvivorPlayerState>())
	{
		PS->InventoryComponent->AddItem(ItemID);
	}

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

void AItemBase::HideForLocalPlayer()
{
	TArray<UActorComponent*> Components;
	GetComponents(Components);

	for (UActorComponent* Comp : Components)
	{
		if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Comp))
		{
			Prim->SetVisibility(false, true);
			Prim->SetRenderInMainPass(false);
			Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AItemBase::SetOutlineEnabled(bool bEnable)
{
	if (!MeshComponent) return;

	MeshComponent->SetRenderCustomDepth(bEnable);

}

//Box Component Range TEST
void AItemBase::DebugDrawCapsule()
{
	//if (!InteractiongBox) return;

	//const FVector Location = InteractiongBox->GetComponentLocation();
	//const FQuat Rotation = InteractiongBox->GetComponentQuat();
	//const FVector Extent = InteractiongBox->GetScaledBoxExtent(); // ÇÙ½É

	//DrawDebugBox(
	//	GetWorld(),
	//	Location,
	//	Extent,
	//	Rotation,
	//	FColor::Green,
	//	false,
	//	0.1f,
	//	0,
	//	2.f
	//);
}



