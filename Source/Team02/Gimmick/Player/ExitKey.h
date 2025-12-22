#pragma once

#include "CoreMinimal.h"
#include "Gimmick/Player/ItemBase.h"
#include "ExitKey.generated.h"

class UStatickMeshComponent;

UCLASS()
class TEAM02_API AExitKey : public AItemBase
{
	GENERATED_BODY()
	
public:	
	AExitKey();

	void CompleteInteract(class AT2PlayerCharacter* Player) override;


protected:
	virtual void BeginPlay() override;

};
