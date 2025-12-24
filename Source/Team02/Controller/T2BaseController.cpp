#include "Controller/T2BaseController.h"
#include "PlayerState/Player/SurvivorPlayerState.h"
#include "UI/UW_KillerHUD.h"
#include "Blueprint/UserWidget.h"
#include "UW_SurvivorHUD.h"
#include "UI/UW_RoundProgressBar.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "Character/T2BaseCharacter.h"
#include "Gimmick/Player/ItemBase.h"
#include "EngineUtils.h"

AT2BaseController::AT2BaseController()
{
}

void AT2BaseController::BeginPlay()
{
    Super::BeginPlay();

    // 서버:  BeginPlay에서 델리게이트 바인딩 시도
    if (HasAuthority() && IsLocalPlayerController())
    {
        BindRoleChangedDelegate();
    }
}

void AT2BaseController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    // 클라이언트: PlayerState가 복제되면 델리게이트 바인딩
    BindRoleChangedDelegate();

    if (AT2PlayerState* PS = GetPlayerState<AT2PlayerState>())
    {
        UE_LOG(LogTemp, Warning, TEXT("OnRep_PlayerState: Role = %d"), (int32)PS->PlayerRole);

        if (PS->PlayerRole != EPlayerRole::None)
        {
            UpdateHUDForRole(PS->PlayerRole);
        }
    }
}

void AT2BaseController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (IsLocalPlayerController())
    {
        BindRoleChangedDelegate();

        if (AT2PlayerState* PS = GetPlayerState<AT2PlayerState>())
        {
            UE_LOG(LogTemp, Warning, TEXT("OnPossess: Role = %d"), (int32)PS->PlayerRole);

            if (PS->PlayerRole != EPlayerRole::None)
            {
                UpdateHUDForRole(PS->PlayerRole);
            }
        }
    }

    Client_ApplyItemVisibility();
}

void AT2BaseController::BindRoleChangedDelegate()
{
    if (bDelegateBound) return;

    if (AT2PlayerState* PS = GetPlayerState<AT2PlayerState>())
    {
        PS->OnPlayerRoleChanged.AddDynamic(this, &AT2BaseController::OnPlayerRoleChanged);
        bDelegateBound = true;
        UE_LOG(LogTemp, Warning, TEXT("BindRoleChangedDelegate:  Delegate bound! "));
    }
}

void AT2BaseController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // ★ ESC 키 바인딩
    InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AT2BaseController::ToggleSettingsMenu);
}

void AT2BaseController::UpdateHUDForRole(EPlayerRole NewRole)
{
    if (!IsLocalPlayerController()) return;
    if (NewRole == EPlayerRole::None) return;
    if (bHUDInitialized && CurrentDisplayedRole == NewRole) return;

    CurrentDisplayedRole = NewRole;
    bHUDInitialized = true;

    UE_LOG(LogTemp, Warning, TEXT("UpdateHUDForRole: %s"),
        NewRole == EPlayerRole::Killer ? TEXT("Killer") : TEXT("Survivor"));

    HideAllHUD();

    if (NewRole == EPlayerRole::Killer)
    {
        ShowKillerHUD();
    }
    else if (NewRole == EPlayerRole::Survivor)
    {
        ShowSurvivorHUD();
    }
}

void AT2BaseController::ShowKillerHUD()
{
    if (!KillerHUDWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("KillerHUDWidgetClass is not set!"));
        return;
    }

    if (!KillerHUDInstance)
    {
        KillerHUDInstance = CreateWidget<UUW_KillerHUD>(this, KillerHUDWidgetClass);
    }

    if (KillerHUDInstance)
    {
        KillerHUDInstance->AddToViewport();
        UE_LOG(LogTemp, Warning, TEXT("KillerHUD displayed!"));
    }
}

void AT2BaseController::ShowSurvivorHUD()
{
    if (!SurvivorHUDWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("SurvivorHUDWidgetClass is not set!"));
        return;
    }

    if (!SurvivorHUDInstance)
    {
        SurvivorHUDInstance = CreateWidget<UUW_SurvivorHUD>(this, SurvivorHUDWidgetClass);
    }

    ASurvivorPlayerState* PS = GetPlayerState<ASurvivorPlayerState>();
    if (!PS) return;

    if (SurvivorHUDInstance)
    {
        SurvivorHUDInstance->AddToViewport();
        SurvivorHUDInstance->Init(PS);
        UE_LOG(LogTemp, Warning, TEXT("SurvivorHUD displayed! "));
    }
}

void AT2BaseController::HideAllHUD()
{
    if (KillerHUDInstance && KillerHUDInstance->IsInViewport())
    {
        KillerHUDInstance->RemoveFromParent();
    }

    if (SurvivorHUDInstance && SurvivorHUDInstance->IsInViewport())
    {
        SurvivorHUDInstance->RemoveFromParent();
    }
}

void AT2BaseController::OnPlayerRoleChanged(EPlayerRole NewRole)
{
    UE_LOG(LogTemp, Warning, TEXT("OnPlayerRoleChanged Delegate Called!  Role: %d"), (int32)NewRole);
    UpdateHUDForRole(NewRole);
}

// ★★★ ESC 설정창 토글 ★★★
void AT2BaseController::ToggleSettingsMenu()
{
    UE_LOG(LogTemp, Warning, TEXT("ESC Pressed - ToggleSettingsMenu"));

    if (!IsLocalPlayerController()) return;

    if (bIsSettingsMenuOpen)
    {
        CloseSettingsMenu();
    }
    else
    {
        OpenSettingsMenu();
    }
}

// ★★★ 설정창 열기 ★★★
void AT2BaseController::OpenSettingsMenu()
{
    if (!SettingsMenuWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("SettingsMenuWidgetClass is not set!"));
        return;
    }

    if (!SettingsMenuInstance)
    {
        SettingsMenuInstance = CreateWidget<UUserWidget>(this, SettingsMenuWidgetClass);
    }

    if (SettingsMenuInstance)
    {
        SettingsMenuInstance->AddToViewport(100);
        bIsSettingsMenuOpen = true;

        // 마우스 커서 표시 + UI 입력 모드 (게임은 멈추지 않음)
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(SettingsMenuInstance->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        SetInputMode(InputMode);
        bShowMouseCursor = true;

        UE_LOG(LogTemp, Warning, TEXT("Settings Menu Opened!"));
    }
}

// ★★★ 설정창 닫기 ★★★
void AT2BaseController::CloseSettingsMenu()
{
    if (SettingsMenuInstance && SettingsMenuInstance->IsInViewport())
    {
        SettingsMenuInstance->RemoveFromParent();
    }

    bIsSettingsMenuOpen = false;

    // 게임 입력 모드로 복귀
    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = false;

    UE_LOG(LogTemp, Warning, TEXT("Settings Menu Closed!"));
}

// ★★★ 게임 나가기 (블루프린트에서 버튼에 연결) ★★★
void AT2BaseController::RequestLeaveGame()
{
    UE_LOG(LogTemp, Warning, TEXT("RequestLeaveGame called! "));

    // 서버(호스트)인 경우 - 나가면 모두 튕김
    if (HasAuthority() && GetNetMode() == NM_ListenServer)
    {
        UE_LOG(LogTemp, Warning, TEXT("Server is leaving - all players will disconnect"));
    }

    // 로비/타이틀로 이동
    if (UWorld* World = GetWorld())
    {
        UGameplayStatics::OpenLevel(World, TEXT("/Game/Library_Pack/Maps/Example"));
    }
}

void AT2BaseController::StartInteractUI()
{
    if (bInteractUIActive) return;
    if (!InteractWidgetClass) return;

    InteractWidgetClassInstance = CreateWidget<UUW_RoundProgressBar>(this, InteractWidgetClass);

    if (!InteractWidgetClassInstance) return;

    InteractWidgetClassInstance->AddToViewport();
    bInteractUIActive = true;

    UImage* ProgressImage = Cast<UImage>(InteractWidgetClassInstance->GetWidgetFromName(TEXT("RoundProgressImage")));

    if (IsValid(ProgressImage) == true)
    {
        InteractMID = ProgressImage->GetDynamicMaterial();
        InteractMID->SetScalarParameterValue(TEXT("Percent"), 0.f);
    }
}

void AT2BaseController::UpdateInteractUI(float Percent)
{
    if (!bInteractUIActive || !InteractMID) return;

    Percent = FMath::Clamp(Percent, 0.f, 1.f);
    InteractMID->SetScalarParameterValue(TEXT("Percent"), Percent);
}

void AT2BaseController::StopInteractUI()
{
    if (!bInteractUIActive) return;

    if (InteractWidgetClassInstance)
    {
        InteractWidgetClassInstance->RemoveFromParent();
        InteractWidgetClassInstance = nullptr;
    }

    InteractMID = nullptr;
    bInteractUIActive = false;
}

void AT2BaseController::Client_ApplyItemVisibility_Implementation()
{
    APawn* PossessedPawn = GetPawn();
    if (!PossessedPawn) return;

    if (!PossessedPawn->ActorHasTag("Killer")) return;

    for (TActorIterator<AItemBase> It(GetWorld()); It; ++It)
    {
        It->HideForLocalPlayer();
    }
}