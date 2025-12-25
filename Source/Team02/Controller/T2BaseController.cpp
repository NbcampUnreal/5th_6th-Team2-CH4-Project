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
#include "GameMode/T2GameModeBase.h" 

AT2BaseController::AT2BaseController()
{
}

void AT2BaseController::BeginPlay()
{
    Super::BeginPlay();

    // 로컬 플레이어면 타이틀 UI 표시
    if (IsLocalPlayerController())
    {
        // 타이틀 맵인지 확인 (Example 맵)
        FString MapName = GetWorld()->GetMapName();
        MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

        if (MapName.Contains(TEXT("Example")) && !MapName.Contains(TEXT("Example1")))
        {
            ShowTitleUI();
        }

        BindRoleChangedDelegate();
    }
}

void AT2BaseController::ShowTitleUI()
{
    if (!IsLocalPlayerController()) return;

    if (!TitleWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("TitleWidgetClass is not set! "));
        return;
    }

    if (!TitleWidgetInstance)
    {
        TitleWidgetInstance = CreateWidget<UUserWidget>(this, TitleWidgetClass);
    }

    if (TitleWidgetInstance)
    {
        TitleWidgetInstance->AddToViewport();

        bShowMouseCursor = true;
        FInputModeUIOnly InputMode;
        SetInputMode(InputMode);

        // 타이틀 카메라 설정
        TArray<AActor*> FoundCameras;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("TitleCamera"), FoundCameras);

        UE_LOG(LogTemp, Warning, TEXT("ShowTitleUI: Found %d TitleCamera(s)"), FoundCameras.Num());

        if (FoundCameras.Num() > 0)
        {
            SetViewTargetWithBlend(FoundCameras[0], 0.0f);
            UE_LOG(LogTemp, Warning, TEXT("ShowTitleUI: Set view to %s"), *FoundCameras[0]->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("ShowTitleUI:  No TitleCamera found!  Add 'TitleCamera' tag to camera actor. "));
        }

        UE_LOG(LogTemp, Warning, TEXT("TitleUI displayed! "));
    }
}

void AT2BaseController::HideTitleUI()
{
    if (TitleWidgetInstance && TitleWidgetInstance->IsInViewport())
    {
        TitleWidgetInstance->RemoveFromParent();
    }
    TitleWidgetInstance = nullptr;
}

void AT2BaseController::TransitionToLobby()
{
    if (!IsLocalPlayerController()) return;

    HideTitleUI();

    // 로비 카메라로 전환
    TArray<AActor*> FoundCameras;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("SelectCamera"), FoundCameras);

    UE_LOG(LogTemp, Warning, TEXT("TransitionToLobby: Found %d SelectCamera(s)"), FoundCameras.Num());

    if (FoundCameras.Num() > 0)
    {
        SetViewTargetWithBlend(FoundCameras[0], 1.0f);
        UE_LOG(LogTemp, Warning, TEXT("TransitionToLobby: Set view to %s"), *FoundCameras[0]->GetName());
    }

    // 로비 UI 표시
    ShowLobbyUI();

    UE_LOG(LogTemp, Warning, TEXT("Transitioned to Lobby! "));
}
void AT2BaseController::ShowLobbyUI()
{
    if (!IsLocalPlayerController()) return;

    if (!LobbyWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("LobbyWidgetClass is not set! "));
        return;
    }

    if (!LobbyWidgetInstance)
    {
        LobbyWidgetInstance = CreateWidget<UUserWidget>(this, LobbyWidgetClass);
    }

    if (LobbyWidgetInstance)
    {
        LobbyWidgetInstance->AddToViewport();

        bShowMouseCursor = true;
        FInputModeUIOnly InputMode;
        SetInputMode(InputMode);

        UE_LOG(LogTemp, Warning, TEXT("LobbyUI displayed!"));
    }
}

void AT2BaseController::HideLobbyUI()
{
    if (LobbyWidgetInstance && LobbyWidgetInstance->IsInViewport())
    {
        LobbyWidgetInstance->RemoveFromParent();
    }
    LobbyWidgetInstance = nullptr;
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
        // 새 Pawn을 Possess하면 바인딩 다시 확인
        BindRoleChangedDelegate();

        if (AT2PlayerState* PS = GetPlayerState<AT2PlayerState>())
        {
            UE_LOG(LogTemp, Warning, TEXT("OnPossess: Role = %d, CurrentDisplayedRole = %d"),
                (int32)PS->PlayerRole, (int32)CurrentDisplayedRole);

            if (PS->PlayerRole != EPlayerRole::None)
            {
                // 새 Pawn Possess 시 항상 HUD 재생성
                HideAllHUD();

                // 기존 HUD 인스턴스 완전히 제거
                if (SurvivorHUDInstance)
                {
                    SurvivorHUDInstance->RemoveFromParent();
                    SurvivorHUDInstance = nullptr;
                }
                if (KillerHUDInstance)
                {
                    KillerHUDInstance->RemoveFromParent();
                    KillerHUDInstance = nullptr;
                }

                bHUDInitialized = false;
                CurrentDisplayedRole = EPlayerRole::None;

                UpdateHUDForRole(PS->PlayerRole);
            }
        }
    }

    Client_ApplyItemVisibility();
}

void AT2BaseController::BindRoleChangedDelegate()
{
    if (!IsLocalPlayerController()) return;

    AT2PlayerState* PS = GetPlayerState<AT2PlayerState>();
    if (!PS) return;

    // 이미 같은 PlayerState에 바인딩되어 있으면 스킵
    if (bDelegateBound && BoundPlayerState == PS)
    {
        return;
    }

    // 다른 PlayerState에 바인딩되어 있으면 해제
    if (bDelegateBound && BoundPlayerState && BoundPlayerState != PS)
    {
        BoundPlayerState->OnPlayerRoleChanged.RemoveDynamic(this, &AT2BaseController::OnPlayerRoleChanged);
        UE_LOG(LogTemp, Warning, TEXT("BindRoleChangedDelegate: Unbound from %s"), *BoundPlayerState->GetName());
        bDelegateBound = false;
    }

    PS->OnPlayerRoleChanged.AddDynamic(this, &AT2BaseController::OnPlayerRoleChanged);
    BoundPlayerState = PS;
    bDelegateBound = true;
    UE_LOG(LogTemp, Warning, TEXT("BindRoleChangedDelegate: Delegate bound to %s"), *PS->GetName());
}

void AT2BaseController::SetupInputComponent()
{
    Super::SetupInputComponent();


    InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AT2BaseController::ToggleSettingsMenu);
}

void AT2BaseController::UpdateHUDForRole(EPlayerRole NewRole)
{
    if (!IsLocalPlayerController())
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateHUDForRole:  Not local controller, skipping"));
        return;
    }

    if (NewRole == EPlayerRole::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateHUDForRole: Role is None, skipping"));
        return;
    }

    // 자기 PlayerState의 Role과 일치하는지 확인
    if (AT2PlayerState* PS = GetPlayerState<AT2PlayerState>())
    {
        if (PS->PlayerRole != NewRole)
        {
            UE_LOG(LogTemp, Warning, TEXT("UpdateHUDForRole: Role mismatch (PS=%d, NewRole=%d), skipping"),
                (int32)PS->PlayerRole, (int32)NewRole);
            return;
        }
    }

    if (bHUDInitialized && CurrentDisplayedRole == NewRole)
    {
        return;
    }

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
    if (!IsLocalPlayerController()) return;

    // 자기 PlayerState의 Role과 일치하는지 확인
    AT2PlayerState* MyPS = GetPlayerState<AT2PlayerState>();
    if (!MyPS)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPlayerRoleChanged: MyPS is null, skipping"));
        return;
    }

    APawn* MyPawn = GetPawn();
    if (!MyPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPlayerRoleChanged: No Pawn yet, skipping"));
        return;
    }


    if (MyPawn->GetClass()->GetName().Contains(TEXT("DefaultPawn")))
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPlayerRoleChanged: Still DefaultPawn, skipping"));
        return;
    }

    if (MyPS->PlayerRole != NewRole)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPlayerRoleChanged: Role mismatch, skipping (MyRole=%d, NewRole=%d)"),
            (int32)MyPS->PlayerRole, (int32)NewRole);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("OnPlayerRoleChanged: Role = %d, Pawn = %s"),
        (int32)NewRole, *MyPawn->GetName());
    UpdateHUDForRole(NewRole);
}


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
void AT2BaseController::ServerRequestStartGame_Implementation()
{
    // 서버에서만 실행
    if (AT2GameModeBase* GM = GetWorld()->GetAuthGameMode<AT2GameModeBase>())
    {
        GM->TryStartGame();
    }
}
void AT2BaseController::Client_OnGameStarted_Implementation()
{
    HideLobbyUI();
    HideTitleUI();

    bShowMouseCursor = false;
    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);

    UE_LOG(LogTemp, Warning, TEXT("Client:  Game Started!"));
}

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