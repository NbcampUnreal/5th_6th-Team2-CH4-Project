#include "Gimmick/Player/FlashlightComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Components/SpotLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "PlayerState/T2PlayerState.h"

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
	if (CurrentBattery > 0.f)
	{
		bIsOn = !bIsOn;
	}

#pragma region Turn ON/OFF DEBUG LOGGING
	/*if (bIsOn)
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Flashlight ON!")), true, true, FLinearColor::Red, 5.f);
	}
	else
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Flashlight OFF!")), true, true, FLinearColor::Red, 5.f);
	}*/
#pragma endregion

	ApplyFlashlightState();   //  서버 / Standalone 화면용
}

void UFlashlightComponent::DrainBattery(float DeltaTime)
{
	CurrentBattery -= DrainPerSecond * DeltaTime;
	CurrentBattery = FMath::Clamp(CurrentBattery, 0.f, MaxBattery);

	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s / Battery : %f"),*GetOwner()->GetName(), CurrentBattery), true, true, FLinearColor::Blue, 5.f);

	if (CurrentBattery <= 0.f)
	{
		bIsOn = false;
		OnRep_FlashlightOn();
	}
}

void UFlashlightComponent::AddBattery(float AddBatteryAmount)
{
	CurrentBattery = FMath::Clamp(CurrentBattery + AddBatteryAmount, 0.f, MaxBattery);

	UE_LOG(LogTemp, Warning, TEXT("Battery Amount : %f"), CurrentBattery);
}

void UFlashlightComponent::OnRep_Battery()
{
	if (!IsValid(this))
	{
		return;
	}

	OnBatteryChanged.Broadcast(CurrentBattery, MaxBattery);
}

void UFlashlightComponent::OnRep_FlashlightOn()
{
	
	ApplyFlashlightState();  // 원격 클라이언트 화면용
}

void UFlashlightComponent::ApplyFlashlightState()
{

	if (!CachedSpotLight) return;

	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!PawnOwner) return;

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (AT2PlayerState* LocalPS = PC->GetPlayerState<AT2PlayerState>())
		{
			if (LocalPS->PlayerRole == EPlayerRole::Killer)
			{
				CachedSpotLight->SetVisibility(false, true);
				CachedSpotLight->SetIntensity(0.f);
				return; 
			}
		}
	}

	CachedSpotLight->SetVisibility(bIsOn);


	if (bIsOn)
	{
		if (PawnOwner->IsLocallyControlled())
		{
			UGameplayStatics::PlaySound2D(this, ToggleSound);

			CachedSpotLight->SetIntensity(50000.f);
			CachedSpotLight->SetAttenuationRadius(1000.f);
		}
		else
		{
			CachedSpotLight->SetIntensity(10000.f);
			CachedSpotLight->SetAttenuationRadius(700.f);
		}
	}
	if (!bIsOn)
	{
		if (PawnOwner->IsLocallyControlled())
		{
			UGameplayStatics::PlaySound2D(this, ToggleSound);
		}
	}
}
