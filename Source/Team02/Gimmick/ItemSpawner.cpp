#include "Gimmick/ItemSpawner.h"
#include "EngineUtils.h"
#include "Engine/TargetPoint.h"

AItemSpawner::AItemSpawner()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AItemSpawner::BeginPlay()
{
    Super::BeginPlay();

}

void AItemSpawner::SpawnItems()
{
    SpawnPoints.Empty();

    for (TActorIterator<ATargetPoint> It(GetWorld()); It; ++It)
    {
        SpawnPoints.Add(*It);
    }

    if (SpawnPoints.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("No TargetPoints"));
        return;
    }

    TArray<AActor*> ShuffledPoints = SpawnPoints;
    ShuffledPoints.Sort([](const AActor& A, const AActor& B) {
        return FMath::RandBool();
        });

    int32 UsedIndex = 0;

    auto SpawnN = [&](TSubclassOf<AActor> ItemClass, int32 Count)
        {
            for (int i = 0; i < Count; i++)
            {
                if (UsedIndex >= ShuffledPoints.Num()) return;

                AActor* Point = ShuffledPoints[UsedIndex++];
                FTransform T = Point->GetActorTransform();

                GetWorld()->SpawnActor<AActor>(ItemClass, T);
            }
        };

    SpawnN(KeyClass, NumKeys);
    SpawnN(PotionClass, NumPotions);
    SpawnN(BatteryClass, NumBatteries);
}




