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


    virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;
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



private:
    //  최대 인원은 유지 
    int32 MaxKillers = 1;
    int32 MaxSurvivors = 4;

    //  현재 카운트 
    int32 CurrentKillers = 0;
    int32 CurrentSurvivors = 0;

    //  총 접속 인원 
    int32 TotalPlayers = 0;

    TMap<APlayerController*, EPlayerRole> PlayerRoles;
    EPlayerRole PendingRole = EPlayerRole::None;
    bool bMatchEnded = false;

    EPlayerRole AssignRoleForNewPlayer(const FString& Options);
    EPlayerRole GetPlayerRoleFromMap(AController* Player);
};