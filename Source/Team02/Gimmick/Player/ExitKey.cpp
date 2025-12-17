#include "Gimmick/Player/ExitKey.h"
#include "Components/CapsuleComponent.h"
#include "Character/PlayerCharacter/T2PlayerCharacter.h"
#include "GameState/T2GameStateBase.h"
#include "Character/KillerCharacter/T2KillerCharacter.h"

// Sets default values
AExitKey::AExitKey()
{
	bReplicates = true;
 	PrimaryActorTick.bCanEverTick = false;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	ExitKey = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExitKey"));
	ExitKey->SetupAttachment(SceneComponent);
	ExitKey->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	OverlapCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("OverlapCapsule"));
	OverlapCapsule->SetupAttachment(SceneComponent);

	OverlapCapsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapCapsule->SetCollisionObjectType(ECC_WorldDynamic);
	OverlapCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	OverlapCapsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	OverlapCapsule->SetGenerateOverlapEvents(true);
}

void AExitKey::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() == NM_Client)
	{
		APawn* LocalPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
		if (LocalPawn && LocalPawn->IsA(AT2KillerCharacter::StaticClass()))
		{
			ExitKey->SetVisibility(false, true);
			OverlapCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	if (HasAuthority())
	{
		OverlapCapsule->OnComponentBeginOverlap.AddDynamic(this, &AExitKey::OnOverlapBegin);
	}

	
}

void AExitKey::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	if (AT2PlayerCharacter* Player = Cast<AT2PlayerCharacter>(OtherActor))
	{
		AT2GameStateBase* GS = GetWorld()->GetGameState<AT2GameStateBase>();
		if (IsValid(GS) == true)
		{
			GS->AddKeyCount(1);
		}

		Destroy();
	}
}



