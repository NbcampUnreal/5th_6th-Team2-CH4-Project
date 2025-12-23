#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "T2GameInstance.h"
#include "T2PlayGameState.h"
#include "T2PlayGameMod.generated.h"

UCLASS()
class TEAM02_API AT2PlayGameMod : public AGameModeBase
{
    GENERATED_BODY()

public:
    AT2PlayGameMod();

    virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

    // ★★★ 팀원이 요청한 함수:  캐릭터 사망 처리 ★★★
    UFUNCTION(BlueprintCallable, Category = "Match")
    void OnCharacterDead(APlayerController* DeadPlayerController);

    UFUNCTION(BlueprintCallable, Category = "Match")
    void OnPlayerDied(APlayerController* Player);

    UFUNCTION(BlueprintCallable, Category = "Match")
    void OnPlayerEscaped(APlayerController* Player);

    void CheckWinConditions();
    void EndMatch(EMatchResult Result);

protected:
    virtual void BeginPlay() override;
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
    virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

    UPROPERTY(EditDefaultsOnly, Category = "Characters")
    TSubclassOf<APawn> KillerClass;

    UPROPERTY(EditDefaultsOnly, Category = "Characters")
    TSubclassOf<APawn> SurvivorClass;

    UPROPERTY(EditDefaultsOnly, Category = "Game")
    int32 RequiredPlayers = 3;

private:
    bool bRolesAssigned = false;
    bool bMatchEnded = false;

    TArray<APlayerController*> ConnectedPlayers;

    void AssignRolesIfReady();
    EPlayerRole GetPlayerRole(AController* Player) const;
};