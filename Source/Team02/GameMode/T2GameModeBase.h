#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "T2GameInstance.h"
#include "Controller/Player/T2PlayerController.h"
#include "Character/PlayerCharacter/T2PlayerCharacter.h"
#include "T2GameModeBase.generated.h"

UCLASS()
class TEAM02_API AT2GameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:
    AT2GameModeBase();

    virtual void BeginPlay() override;

    virtual void PostLogin(APlayerController* NewPlayer) override;

    virtual void Logout(AController* Exiting) override;

protected:
    void TryStartCountdown();

    void CancelCountdown();

    void TickCountdown();

    FTimerHandle CountdownTimer;

    bool bCountdownRunning = false;

    int32 CountdownLeft;

    UPROPERTY(EditDefaultsOnly)
    int32 RequiredPlayers = 3;

    UPROPERTY(EditDefaultsOnly)
    FName MainLevelName = TEXT("Example1");



//    // 역할 선택 화면으로 전환
//    UFUNCTION(BlueprintCallable, Category = "UI")
//    void TransitionToRoleSelect();
//
//    // 역할 선택 후 게임 시작
//    UFUNCTION(BlueprintCallable, Category = "Game")
//    void StartGameAsKiller();
//
//    UFUNCTION(BlueprintCallable, Category = "Game")
//    void StartGameAsSurvivor();
//
//    void OnPlayerDead(AT2PlayerCharacter* DeadCharacter);
//
//protected:
//    virtual void BeginPlay() override;
//
//    UPROPERTY(EditDefaultsOnly, Category = "UI")
//    TSubclassOf<UUserWidget> TitleWidgetClass;
//
//    UPROPERTY(EditDefaultsOnly, Category = "UI")
//    TSubclassOf<UUserWidget> RoleSelectWidgetClass;
//
//
//    UPROPERTY(EditDefaultsOnly, Category = "Game")
//    FName GamePlayMapName = "Example1";
//
//
//    UPROPERTY(EditDefaultsOnly, Category = "Camera")
//    float CameraBlendTime = 1.0f;
//
//private:
//    UPROPERTY()
//    UUserWidget* CurrentWidget;
//
//    AActor* FindCameraByTag(FName Tag);
//    void SwitchWidget(TSubclassOf<UUserWidget> NewWidgetClass);
//    void StartGameWithRole(EPlayerRole InRole);
};