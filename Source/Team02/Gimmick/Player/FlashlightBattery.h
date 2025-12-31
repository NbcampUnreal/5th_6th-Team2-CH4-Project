#pragma once

#include "CoreMinimal.h"
#include "Gimmick/Player/ItemBase.h"
#include "FlashlightBattery.generated.h"

UCLASS()
class TEAM02_API AFlashlightBattery : public AItemBase
{
	GENERATED_BODY()
	
public:	
	AFlashlightBattery();

protected:
	virtual void BeginPlay() override;

	

};
