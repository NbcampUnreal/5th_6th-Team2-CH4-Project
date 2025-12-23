#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "T2GameInstance.h"
#include "Character/PlayerCharacter/T2PlayerCharacter.h"
#include "T2GameModeBase.generated.h"

UCLASS()
class TEAM02_API AT2GameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:
    AT2GameModeBase();

    // Start 버튼 → 로비 화면으로 전환
    UFUNCTION(BlueprintCallable, Category = "UI")
    void TransitionToLobby();

    // 현재 접속 인원 수 반환
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    int32 GetCurrentPlayerCount();

    // 게임 시작 가능 여부 (3명 체크)
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    bool CanStartGame();

    // 호스트가 게임 시작 버튼 클릭 시 호출
    UFUNCTION(BlueprintCallable, Category = "Game")
    void TryStartGame();

    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

    void OnPlayerDead(AT2PlayerCharacter* DeadCharacter);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> TitleWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> LobbyWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "Game")
    FName GamePlayMapName = "Example1";

    UPROPERTY(EditDefaultsOnly, Category = "Game")
    int32 RequiredPlayers = 3;

    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float CameraBlendTime = 1.0f;

private:
    UPROPERTY()
    UUserWidget* CurrentWidget;

    AActor* FindCameraByTag(FName Tag);
    void SwitchWidget(TSubclassOf<UUserWidget> NewWidgetClass);
};