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

    if (IsLocalPlayerController())
    {
        FString MapName = GetWorld()->GetMapName();
        MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);


        if (MapName.Contains(TEXT("title")))
        {
            ShowTitleUI();
        }

        BindRoleChangedDelegate();
    }
}

void AT2BaseController::ShowTitleUI()
{
    if (!IsLocalPlayerController()) return;

    UE_LOG(LogTemp, Warning, TEXT("ShowTitleUI:  Started"));

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

        // Ÿ��Ʋ ī�޶� ����
        TArray<AActor*> FoundCameras;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("TitleCamera"), FoundCameras);

        UE_LOG(LogTemp, Warning, TEXT("ShowTitleUI: Found %d TitleCamera(s)"), FoundCameras.Num());

        if (FoundCameras.Num() > 0)
        {
            AActor* CameraActor = FoundCameras[0];
            UE_LOG(LogTemp, Warning, TEXT("ShowTitleUI:  Camera Actor = %s"), *CameraActor->GetName());

            // Pawn üũ
            APawn* MyPawn = GetPawn();
            UE_LOG(LogTemp, Warning, TEXT("ShowTitleUI:  MyPawn = %s"), MyPawn ? *MyPawn->GetName() : TEXT("NULL"));

            SetViewTargetWithBlend(CameraActor, 0.0f);
            UE_LOG(LogTemp, Warning, TEXT("ShowTitleUI:  SetViewTargetWithBlend called"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("ShowTitleUI:  No TitleCamera found!"));
        }

        UE_LOG(LogTemp, Warning, TEXT("TitleUI displayed!"));
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

    // �κ� ī�޶�� ��ȯ
    TArray<AActor*> FoundCameras;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("SelectCamera"), FoundCameras);

    UE_LOG(LogTemp, Warning, TEXT("TransitionToLobby: Found %d SelectCamera(s)"), FoundCameras.Num());

    if (FoundCameras.Num() > 0)
    {
        SetViewTargetWithBlend(FoundCameras[0], 1.0f);
        UE_LOG(LogTemp, Warning, TEXT("TransitionToLobby: Set view to %s"), *FoundCameras[0]->GetName());
    }

    // �κ� UI ǥ��
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

    // Ŭ���̾�Ʈ: PlayerState�� �����Ǹ� ��������Ʈ ���ε�
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
        // �� Pawn�� Possess�ϸ� ���ε� �ٽ� Ȯ��
        BindRoleChangedDelegate();

        if (AT2PlayerState* PS = GetPlayerState<AT2PlayerState>())
        {
            UE_LOG(LogTemp, Warning, TEXT("OnPossess: Role = %d, CurrentDisplayedRole = %d"),
                (int32)PS->PlayerRole, (int32)CurrentDisplayedRole);

            if (PS->PlayerRole != EPlayerRole::None)
            {
                // �� Pawn Possess �� �׻� HUD �����
                HideAllHUD();

                // ���� HUD �ν��Ͻ� ������ ����
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
}

void AT2BaseController::BindRoleChangedDelegate()
{
    if (!IsLocalPlayerController()) return;

    AT2PlayerState* PS = GetPlayerState<AT2PlayerState>();
    if (!PS) return;

    // �̹� ���� PlayerState�� ���ε��Ǿ� ������ ��ŵ
    if (bDelegateBound && BoundPlayerState == PS)
    {
        return;
    }

    // �ٸ� PlayerState�� ���ε��Ǿ� ������ ����
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

    // �ڱ� PlayerState�� Role�� ��ġ�ϴ��� Ȯ��
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

    // �ڱ� PlayerState�� Role�� ��ġ�ϴ��� Ȯ��
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

        // ���콺 Ŀ�� ǥ�� + UI �Է� ��� (������ ������ ����)
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
    // ���������� ����
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

    // ���� �Է� ���� ����
    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = false;

    UE_LOG(LogTemp, Warning, TEXT("Settings Menu Closed!"));
}

void AT2BaseController::RequestLeaveGame()
{
    UE_LOG(LogTemp, Warning, TEXT("RequestLeaveGame called!"));

    if (HasAuthority() && GetNetMode() == NM_ListenServer)
    {
        UE_LOG(LogTemp, Warning, TEXT("Server is leaving - all players will disconnect"));
    }
    if (UWorld* World = GetWorld())
    {
        UGameplayStatics::OpenLevel(World, TEXT("/Game/Team02/Blueprint/map/title"));
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