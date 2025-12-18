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

    virtual void BeginPlay() override;

    virtual void PostLogin(APlayerController* NewPlayer) override;

    virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

    virtual void PostSeamlessTravel() override;

    void AssignRolesIfReady();

    void OnSurvivorDied();

    void OnKeyCollected();

protected:
    UPROPERTY(EditDefaultsOnly)
    int32 RequiredPlayers = 3;

    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<APawn> KillerPawnClass;

    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<APawn> SurvivorPawnClass;

    bool bRolesAssigned = false;



    /*virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

    virtual void Logout(AController* Exiting) override;
    UFUNCTION(BlueprintCallable, Category = "Match")
    void OnPlayerDied(APlayerController* Player);

    UFUNCTION(BlueprintCallable, Category = "Match")
    void OnPlayerEscaped(APlayerController* Player);
    void CheckWinConditions();      
    void EndMatch(EMatchResult Result);
protected:
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;


    UPROPERTY(EditDefaultsOnly, Category = "Characters")
    TSubclassOf<APawn> KillerClass;

    UPROPERTY(EditDefaultsOnly, Category = "Characters")
    TSubclassOf<APawn> SurvivorClass;


    UPROPERTY(EditDefaultsOnly, Category = "Controllers")
    TSubclassOf<APlayerController> KillerControllerClass;

    UPROPERTY(EditDefaultsOnly, Category = "Controllers")
    TSubclassOf<APlayerController> SurvivorControllerClass;
    bool bMatchEnded = false;
private:
    int32 MaxKillers = 1;
    int32 MaxSurvivors = 4;
    int32 CurrentKillers = 0;
    int32 CurrentSurvivors = 0;
    TMap<APlayerController*, EPlayerRole> PlayerRoles;

    EPlayerRole AssignRoleForNewPlayer(const FString& Options);
    EPlayerRole GetPlayerRoleFromMap(AController* Player);
    EPlayerRole PendingRole = EPlayerRole::None;*/
};