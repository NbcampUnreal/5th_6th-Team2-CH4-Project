#include "PlayerState/Player/InventoryComponent.h"
#include "PlayerState/Player/SurvivorPlayerState.h"
#include "InventoryComponent.h"
#include "Blueprint/UserWidget.h"
#include "Controller/T2BaseController.h"
#include "Character/PlayerCharacter/T2PlayerCharacter.h"
#include "Gimmick/Player/FlashlightComponent.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	Items.SetNum(InventorySize);

	if (InventoryWidgetClass)
	{
		AT2BaseController* PlayerController = Cast<AT2BaseController>(GetOwner()->GetInstigatorController());

		if (PlayerController)
		{
			InventoryWidget = CreateWidget<UUserWidget>(PlayerController, InventoryWidgetClass);
		}
	}
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, Items);
}

void UInventoryComponent::AddItem(FName ItemID)
{
	if (ItemID == NAME_None)
	{
		return;
	}

	for (int32 i = 0; i < Items.Num(); i++)
	{
		if (Items[i].ItemID == NAME_None)
		{
			Items[i].ItemID = ItemID;

			UE_LOG(LogTemp, Warning, TEXT("Server added %s to slot %d"), *ItemID.ToString(), i);

			OnInventoryUpdated.Broadcast();

			return;
		}
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Inventory is full"));
	}
}

void UInventoryComponent::UseSlot(int32 SlotIndex)
{
	if (!Items.IsValidIndex(SlotIndex))
	{
		return;
	}

	FName ItemID = Items[SlotIndex].ItemID;


	if (ItemID == NAME_None) return;

	ASurvivorPlayerState* PS = Cast<ASurvivorPlayerState>(GetOwner());
	if (!PS) return;

	AT2PlayerCharacter* T2PC = Cast<AT2PlayerCharacter>(PS->GetPawn());
	if (!T2PC) return;


	if (ItemID == "Potion")
	{
		PS->ApplyHealByItem(40.f);
		T2PC->Client_UsePotionEffect();

		Items[SlotIndex].ItemID = NAME_None;
		
	}
	else if (ItemID == "Battery")
	{
		UFlashlightComponent* FL = T2PC->FindComponentByClass<UFlashlightComponent>();
		if (IsValid(FL) == true)
		{
			FL->AddBattery(30.f);
			T2PC->Client_ChangeBatteryEffect();
		}
		
		Items[SlotIndex].ItemID = NAME_None;
		
	}
	else if (ItemID == "Web")
	{

		// Web >>> Use Item;
	}


	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::OnRep_Items()
{
	OnInventoryUpdated.Broadcast();
}



