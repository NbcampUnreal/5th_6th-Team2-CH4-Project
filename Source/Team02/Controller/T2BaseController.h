#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "T2GameInstance.h"
#include "T2BaseController.generated.h"

class UUserWidget;
class UUW_KillerHUD;
class UUW_SurvivorHUD;
class UUW_RoundProgressBar;

UCLASS()
class TEAM02_API AT2BaseController : public APlayerController
{
    GENERATED_BODY()

public:
    AT2BaseController();

    virtual void BeginPlay() override;
    virtual void OnRep_PlayerState() override;
    virtual void OnPossess(APawn* InPawn) override;

    UFUNCTION(BlueprintCallable, Category = "UI")
    void UpdateHUDForRole(EPlayerRole NewRole);

    // ★ ESC 설정창 토글
    UFUNCTION(BlueprintCallable, Category = "UI")
    void ToggleSettingsMenu();

    // ★ 설정창 열기/닫기
    UFUNCTION(BlueprintCallable, Category = "UI")
    void OpenSettingsMenu();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void CloseSettingsMenu();

    // ★ 게임 나가기 (블루프린트에서 호출)
    UFUNCTION(BlueprintCallable, Category = "Game")
    void RequestLeaveGame();

    void StartInteractUI();
    void UpdateInteractUI(float Percent);
    void StopInteractUI();

    bool bInteractUIActive = false;

    UFUNCTION(Client, Reliable)
    void Client_ApplyItemVisibility();

protected:
    virtual void SetupInputComponent() override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI|Killer")
    TSubclassOf<UUW_KillerHUD> KillerHUDWidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Killer")
    TObjectPtr<UUW_KillerHUD> KillerHUDInstance;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI|Survivor")
    TSubclassOf<UUW_SurvivorHUD> SurvivorHUDWidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Survivor")
    TObjectPtr<UUW_SurvivorHUD> SurvivorHUDInstance;

    // ★ ESC 설정창 위젯
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI|Settings")
    TSubclassOf<UUserWidget> SettingsMenuWidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Settings")
    TObjectPtr<UUserWidget> SettingsMenuInstance;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Settings")
    bool bIsSettingsMenuOpen = false;

public:
    UPROPERTY(EditDefaultsOnly, Category = "UI|Survivor")
    TSubclassOf<UUW_RoundProgressBar> InteractWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI|Survivor")
    TObjectPtr<UUW_RoundProgressBar> InteractWidgetClassInstance;

    UPROPERTY()
    UMaterialInstanceDynamic* InteractMID;

private:
    void ShowKillerHUD();
    void ShowSurvivorHUD();
    void HideAllHUD();
    void BindRoleChangedDelegate();

    UFUNCTION()
    void OnPlayerRoleChanged(EPlayerRole NewRole);

    EPlayerRole CurrentDisplayedRole = EPlayerRole::None;
    bool bHUDInitialized = false;
    bool bDelegateBound = false;
};