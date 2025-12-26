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

    // Start ��ư �� �κ� ȭ������ ��ȯ
    UFUNCTION(BlueprintCallable, Category = "UI")
    void TransitionToLobby();

    // ���� ���� �ο� �� ��ȯ
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    int32 GetCurrentPlayerCount();

    // ���� ���� ���� ���� (3�� üũ)
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    bool CanStartGame();

    // ȣ��Ʈ�� ���� ���� ��ư Ŭ�� �� ȣ��
    UFUNCTION(BlueprintCallable, Category = "Game")
    void TryStartGame();

    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

    void OnPlayerDead(AT2PlayerCharacter* DeadCharacter);
    virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> TitleWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> LobbyWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game")
    FName GamePlayMapName = FName("MedievalVillageNight");

    UPROPERTY(EditDefaultsOnly, Category = "Game")
    int32 RequiredPlayers = 3;

    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float CameraBlendTime = 1.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Lobby")
    bool bInLobby = true;
    bool bIsTitleMap = false;
private:
    UPROPERTY()
    UUserWidget* CurrentWidget;

    AActor* FindCameraByTag(FName Tag);
    void SwitchWidget(TSubclassOf<UUserWidget> NewWidgetClass);

#pragma region potal system
public:
    void OnKeyCollected(int32 CurrentTotalKeys);

protected:
    UPROPERTY(EditAnywhere, Category = "Gimmick")
    TSubclassOf<class APortalActor> PortalClass;

    UPROPERTY(EditAnywhere, Category = "Gimmick")
    int32 KeysRequiredForPortal = 6;

    UPROPERTY(EditAnywhere, Category = "Gimmick")
    float PortalDuration = 120.0f; // 2분

private:
    void SpawnPortalAtRandomLocation();
    
#pragma endregion 
};