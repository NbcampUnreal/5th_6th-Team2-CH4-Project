#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "T2GameInstance.generated.h"

UENUM(BlueprintType)
enum class EPlayerRole : uint8
{
    None,
    Killer,
    Survivor
};

UCLASS()
class TEAM02_API UT2GameInstance : public UGameInstance
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "Game")
    EPlayerRole SelectedRole = EPlayerRole::None;
};