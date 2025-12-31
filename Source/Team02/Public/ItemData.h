#pragma once 

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Gimmick/Player/ItemBase.h"
#include "ItemData.generated.h"

USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
	TSubclassOf<AItemBase> ItemActorClass;
};