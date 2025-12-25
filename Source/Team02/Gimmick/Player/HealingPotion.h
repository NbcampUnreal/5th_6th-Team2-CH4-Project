#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HealingPotion.generated.h"

UCLASS()
class TEAM02_API AHealingPotion : public AActor
{
	GENERATED_BODY()
	
public:	
	AHealingPotion();

protected:
	virtual void BeginPlay() override;


};
