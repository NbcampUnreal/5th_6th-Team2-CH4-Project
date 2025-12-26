#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmick/Player/ItemBase.h"
#include "ItemSpawner.generated.h"

UCLASS()
class TEAM02_API AItemSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AItemSpawner();

    void SpawnItems();

    UPROPERTY(EditAnywhere)
    TArray<AActor*> SpawnPoints;

    UPROPERTY(EditAnywhere)
    TSubclassOf<AActor> KeyClass;

    UPROPERTY(EditAnywhere)
    TSubclassOf<AActor> BatteryClass;

    UPROPERTY(EditAnywhere)
    TSubclassOf<AActor> PotionClass;

    UPROPERTY(EditAnywhere)
    int32 NumKeys = 6;

    UPROPERTY(EditAnywhere)
    int32 NumBatteries = 5;

    UPROPERTY(EditAnywhere)
    int32 NumPotions = 5;

protected:
	virtual void BeginPlay() override;

};
