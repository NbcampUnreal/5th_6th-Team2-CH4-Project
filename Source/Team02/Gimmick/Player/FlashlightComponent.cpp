#include "Gimmick/Player/FlashlightComponent.h"
#include "Kismet/KismetSystemLibrary.h"

UFlashlightComponent::UFlashlightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UFlashlightComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called every frame
void UFlashlightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (bIsOn && CurrentBattery > 0.f)
	{
		DrainBattery(DeltaTime);
	}

}

void UFlashlightComponent::ToggleFlashlight()
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

	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Battery : %f"), CurrentBattery), true, true, FLinearColor::Blue, 5.f);

	if (CurrentBattery <= 0.f)
	{
		bIsOn = false;
	}
}

void UFlashlightComponent::AddBattery()
{
}

