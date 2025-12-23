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

    // ★ 추가
    virtual void OnPossess(APawn* InPawn) override;

    // Role에 따라 HUD를 표시/전환
    UFUNCTION(BlueprintCallable, Category = "UI")
    void UpdateHUDForRole(EPlayerRole NewRole);

    // ESC 설정창 토글
    UFUNCTION(BlueprintCallable, Category = "UI")
    void ToggleSettingsMenu();

    void StartInteractUI();
    void UpdateInteractUI(float Percent);
    void StopInteractUI();

    bool bInteractUIActive = false;

protected:
    virtual void SetupInputComponent() override;

    // Killer HUD
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI|Killer")
    TSubclassOf<UUW_KillerHUD> KillerHUDWidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Killer")
    TObjectPtr<UUW_KillerHUD> KillerHUDInstance;

    // Survivor HUD (UW_SurvivorHUD 클래스 사용)
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI|Survivor")
    TSubclassOf<UUserWidget> SurvivorHUDWidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Survivor")
    TObjectPtr<UUserWidget> SurvivorHUDInstance;

    // Settings Menu (ESC)
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

    // PlayerState의 Role 변경 시 호출
    UFUNCTION()
    void OnPlayerRoleChanged(EPlayerRole NewRole);

    // 현재 적용된 Role 캐시
    EPlayerRole CurrentDisplayedRole = EPlayerRole::None;

    // ★ 추가
    bool bHUDInitialized = false;
};