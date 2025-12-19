#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlashlightBattery.generated.h"

UCLASS()
class TEAM02_API AFlashlightBattery : public AActor
{
	GENERATED_BODY()
	
public:	
	AFlashlightBattery();

protected:
	virtual void BeginPlay() override;

};
