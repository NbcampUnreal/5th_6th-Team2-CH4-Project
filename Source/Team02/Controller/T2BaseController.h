#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "T2GameInstance.h"
#include "T2BaseController.generated.h"

class UUserWidget;
class UUW_KillerHUD;
class UUW_SurvivorHUD;
class UUW_RoundProgressBar;
class UUW_PersonalResult;
class AT2PlayerState;
class AAT2SpectatorPawn;

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

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ToggleSettingsMenu();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void OpenSettingsMenu();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void CloseSettingsMenu();

    UFUNCTION(BlueprintCallable, Category = "Game")
    void RequestLeaveGame();

    void StartInteractUI();
    void UpdateInteractUI(float Percent);
    void StopInteractUI();

    bool bInteractUIActive = false;

    // ========== Result & Spectate System ==========
    
    // Show personal result (Client RPC)
    UFUNCTION(Client, Reliable)
    void Client_ShowPersonalResult(bool bEscaped);

    // Show match end result (Client RPC)
    UFUNCTION(Client, Reliable)
    void Client_ShowMatchResult(bool bSurvivorWin);

    // Start spectating (called from UI button)
    UFUNCTION(BlueprintCallable, Category = "Spectate")
    void StartSpectating();

    // Server RPC to spawn and possess spectator pawn
    UFUNCTION(Server, Reliable)
    void Server_RequestSpectate();

    // Client RPC to notify spectating started
    UFUNCTION(Client, Reliable)
    void Client_OnSpectateStarted();

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

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI|Settings")
    TSubclassOf<UUserWidget> SettingsMenuWidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Settings")
    TObjectPtr<UUserWidget> SettingsMenuInstance;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Settings")
    bool bIsSettingsMenuOpen = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI|Title")
    TSubclassOf<UUserWidget> TitleWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> TitleWidgetInstance;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI|Lobby")
    TSubclassOf<UUserWidget> LobbyWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> LobbyWidgetInstance;

    // Personal result widget class
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI|Result")
    TSubclassOf<UUW_PersonalResult> PersonalResultWidgetClass;

    UPROPERTY()
    TObjectPtr<UUW_PersonalResult> PersonalResultInstance;

    // Spectator Pawn class
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spectator")
    TSubclassOf<AAT2SpectatorPawn> SpectatorPawnClass;

public:
    UPROPERTY(EditDefaultsOnly, Category = "UI|Survivor")
    TSubclassOf<UUW_RoundProgressBar> InteractWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI|Survivor")
    TObjectPtr<UUW_RoundProgressBar> InteractWidgetClassInstance;

    UPROPERTY()
    UMaterialInstanceDynamic* InteractMID;

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowTitleUI();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void HideTitleUI();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void TransitionToLobby();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowLobbyUI();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void HideLobbyUI();

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Game")
    void ServerRequestStartGame();

    UFUNCTION(Client, Reliable)
    void Client_OnGameStarted();

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

    UPROPERTY()
    TObjectPtr<AT2PlayerState> BoundPlayerState;

    // ========== Spectate System ==========
    
    // Get alive survivor pawn (server only)
    APawn* GetAliveTarget();

    // Is spectating
    bool bIsSpectating = false;

    // Was escaped (for match result display)
    bool bWasEscaped = false;

    // Current spawned SpectatorPawn
    UPROPERTY()
    TObjectPtr<AAT2SpectatorPawn> SpawnedSpectatorPawn;
};
