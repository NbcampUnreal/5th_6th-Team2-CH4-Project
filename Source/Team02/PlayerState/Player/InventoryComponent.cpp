#include "PlayerState/Player/InventoryComponent.h"
#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();


}

void UInventoryComponent::AddItem(TSubclassOf<AItemBase> ItemClass)
{
}

void UInventoryComponent::UseItemByIndex(int32 Index)
{
 
}

void UInventoryComponent::Server_UseItem_Implementation(int32 Index)
{
	UseItemByIndex(Index);
}





