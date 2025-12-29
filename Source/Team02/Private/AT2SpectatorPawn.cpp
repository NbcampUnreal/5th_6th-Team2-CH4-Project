#include "Public/AT2SpectatorPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Character/PlayerCharacter/T2PlayerCharacter.h"
#include "PlayerState/Player/SurvivorPlayerState.h"

AAT2SpectatorPawn::AAT2SpectatorPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root scene component
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(Root);

	// Create SpringArm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 400.0f;
	SpringArm->SocketOffset = FVector(0.0f, 0.0f, 150.0f);
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bDoCollisionTest = true;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;
	SpringArm->SetRelativeRotation(FRotator(-25.0f, 0.0f, 0.0f));  // 생성자에서 미리 설정

	// Create Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;

	// Disable auto possess
	AutoPossessPlayer = EAutoReceiveInput::Disabled;
	AutoPossessAI = EAutoPossessAI::Disabled;
}

void AAT2SpectatorPawn::BeginPlay()
{
	Super::BeginPlay();

	// Set initial camera angle
	SpringArm->SetRelativeRotation(FRotator(CurrentPitch, CurrentYaw, 0.0f));

	// Activate camera
	if (Camera)
	{
		Camera->SetActive(true);
	}

	// Set this pawn as view target
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetViewTargetWithBlend(this, 0.0f);
		UE_LOG(LogTemp, Warning, TEXT("SpectatorPawn BeginPlay: SetViewTargetWithBlend called"));
	}

	// If no target set yet, try to find one
	if (!SpectateTarget)
	{
		FindAndSetTarget();
	}
}

void AAT2SpectatorPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Follow spectate target
	if (SpectateTarget && IsValid(SpectateTarget))
	{
		FVector TargetLocation = SpectateTarget->GetActorLocation();
		SetActorLocation(TargetLocation);
		bLoggedTargetInvalid = false; // Reset log flag
	}
	else
	{
		// Try to find a new target if current is invalid
		if (!bLoggedTargetInvalid)
		{
			UE_LOG(LogTemp, Warning, TEXT("SpectatorPawn Tick: SpectateTarget is invalid, trying to find new target..."));
			bLoggedTargetInvalid = true;
		}
		
		// Try to find new target every second
		TimeSinceLastTargetSearch += DeltaTime;
		if (TimeSinceLastTargetSearch >= 1.0f)
		{
			TimeSinceLastTargetSearch = 0.0f;
			FindAndSetTarget();
		}
	}
}

void AAT2SpectatorPawn::FindAndSetTarget()
{
	UE_LOG(LogTemp, Warning, TEXT("SpectatorPawn: FindAndSetTarget called"));

	// Find all T2PlayerCharacter actors
	TArray<AActor*> FoundCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AT2PlayerCharacter::StaticClass(), FoundCharacters);

	UE_LOG(LogTemp, Warning, TEXT("SpectatorPawn: Found %d player characters"), FoundCharacters.Num());

	for (AActor* Actor : FoundCharacters)
	{
		AT2PlayerCharacter* PlayerChar = Cast<AT2PlayerCharacter>(Actor);
		if (!PlayerChar) continue;

		// Skip if pending kill or being destroyed
		if (!IsValid(PlayerChar) || PlayerChar->IsPendingKillPending())
		{
			UE_LOG(LogTemp, Warning, TEXT("SpectatorPawn: %s is pending kill, skipping"), *PlayerChar->GetName());
			continue;
		}

		// Skip hidden actors (dead/escaped players)
		if (PlayerChar->IsHidden()) 
		{
			UE_LOG(LogTemp, Warning, TEXT("SpectatorPawn: %s is hidden, skipping"), *PlayerChar->GetName());
			continue;
		}

		// Skip actors with no collision (dead players)
		if (!PlayerChar->GetActorEnableCollision())
		{
			UE_LOG(LogTemp, Warning, TEXT("SpectatorPawn: %s has no collision (dead), skipping"), *PlayerChar->GetName());
			continue;
		}

		// Try to get PlayerState
		APlayerState* PS = PlayerChar->GetPlayerState();
		if (!PS)
		{
			UE_LOG(LogTemp, Warning, TEXT("SpectatorPawn: %s has no PlayerState"), *PlayerChar->GetName());
			continue;
		}

		// Check if it's a survivor
		ASurvivorPlayerState* SurvivorPS = Cast<ASurvivorPlayerState>(PS);
		if (!SurvivorPS)
		{
			// Not a survivor (probably killer), skip
			UE_LOG(LogTemp, Warning, TEXT("SpectatorPawn: %s is not a Survivor"), *PlayerChar->GetName());
			continue;
		}

		// Check if alive and not escaped
		if (!SurvivorPS->bIsDead && !SurvivorPS->bIsEscaped)
		{
			SpectateTarget = PlayerChar;
			SetActorLocation(PlayerChar->GetActorLocation());
			UE_LOG(LogTemp, Warning, TEXT("SpectatorPawn: Now spectating %s at %s"), 
				*PlayerChar->GetName(), *PlayerChar->GetActorLocation().ToString());
			return;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SpectatorPawn: %s is dead=%s, escaped=%s"), 
				*PlayerChar->GetName(),
				SurvivorPS->bIsDead ? TEXT("true") : TEXT("false"),
				SurvivorPS->bIsEscaped ? TEXT("true") : TEXT("false"));
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("SpectatorPawn: No valid spectate target found!"));
}

void AAT2SpectatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// DO NOT call Super - we don't want any default movement input bindings

	// Only bind mouse look - NO WASD movement
	PlayerInputComponent->BindAxis("Turn", this, &AAT2SpectatorPawn::HandleLookYaw);
	PlayerInputComponent->BindAxis("LookUp", this, &AAT2SpectatorPawn::HandleLookPitch);

	UE_LOG(LogTemp, Warning, TEXT("SpectatorPawn: Input bindings set up (Mouse only)"));
}

void AAT2SpectatorPawn::SetSpectateTarget(AActor* NewTarget)
{
	SpectateTarget = NewTarget;
	
	if (SpectateTarget)
	{
		SetActorLocation(SpectateTarget->GetActorLocation());
		UE_LOG(LogTemp, Warning, TEXT("SpectatorPawn SetSpectateTarget: Now spectating %s at %s"), 
			*SpectateTarget->GetName(), 
			*SpectateTarget->GetActorLocation().ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SpectatorPawn SetSpectateTarget: Target is NULL, will search in Tick"));
	}
}

void AAT2SpectatorPawn::HandleLookYaw(float Value)
{
	if (FMath::IsNearlyZero(Value)) return;

	CurrentYaw += Value * MouseSensitivity;
	SpringArm->SetRelativeRotation(FRotator(CurrentPitch, CurrentYaw, 0.0f));
}

void AAT2SpectatorPawn::HandleLookPitch(float Value)
{
	if (FMath::IsNearlyZero(Value)) return;

	CurrentPitch = FMath::Clamp(CurrentPitch - Value * MouseSensitivity, MinPitch, MaxPitch);
	SpringArm->SetRelativeRotation(FRotator(CurrentPitch, CurrentYaw, 0.0f));
}
