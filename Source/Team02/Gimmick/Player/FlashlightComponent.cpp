#include "Gimmick/Player/FlashlightComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Components/SpotLightComponent.h"

UFlashlightComponent::UFlashlightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UFlashlightComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (IsValid(Owner) == true)
	{
		CachedSpotLight = Owner->FindComponentByClass<USpotLightComponent>();
	}
	
}

void UFlashlightComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UFlashlightComponent, bIsOn);
	DOREPLIFETIME(UFlashlightComponent, CurrentBattery);
}


// Called every frame
void UFlashlightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (GetOwner()->HasAuthority() && bIsOn && CurrentBattery > 0.f)
	{
		DrainBattery(DeltaTime);
	}

}

void UFlashlightComponent::Server_ToggleFlashlight_Implementation()
{
	bIsOn = !bIsOn;

	if (bIsOn)
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Flashlight ON!")), true, true, FLinearColor::Red, 5.f);
	}
	else
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Flashlight OFF!")), true, true, FLinearColor::Red, 5.f);
	}
}

void UFlashlightComponent::DrainBattery(float DeltaTime)
{
	CurrentBattery -= DrainPerSecond * DeltaTime;
	CurrentBattery = FMath::Clamp(CurrentBattery, 0.f, MaxBattery);

	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s / Battery : %f"),*GetOwner()->GetName(), CurrentBattery), true, true, FLinearColor::Blue, 5.f);

	if (CurrentBattery <= 0.f)
	{
		bIsOn = false;
		OnRep_FlashlightOn();
	}
}

void UFlashlightComponent::AddBattery()
{
}

void UFlashlightComponent::OnRep_Battery()
{
	// UI
}

void UFlashlightComponent::OnRep_FlashlightOn()
{
	if (!CachedSpotLight) return;

	CachedSpotLight->SetVisibility(bIsOn);

	APawn* PawnOnwer = Cast<APawn>(GetOwner());
	
	if (!PawnOnwer) return;
	
	if(bIsOn)
	{
		if (PawnOnwer->IsLocallyControlled())
		{
			CachedSpotLight->SetIntensity(50000.f);
			CachedSpotLight->SetAttenuationRadius(1000.f);
		}
		else
		{
			CachedSpotLight->SetIntensity(10000.f);
			CachedSpotLight->SetAttenuationRadius(700.f);
		}
	}
}
