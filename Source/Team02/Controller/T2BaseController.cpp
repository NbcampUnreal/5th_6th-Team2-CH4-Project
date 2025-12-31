#include "Controller/T2BaseController.h"
#include "PlayerState/Player/SurvivorPlayerState.h"
#include "UI/UW_KillerHUD.h"
#include "Blueprint/UserWidget.h"
#include "UW_SurvivorHUD.h"
#include "UI/UW_RoundProgressBar.h"
#include "Public/UW_PersonalResult.h"
#include "Public/AT2SpectatorPawn.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "Character/T2BaseCharacter.h"
#include "Character/PlayerCharacter/T2PlayerCharacter.h"
#include "Gimmick/Player/ItemBase.h"
#include "EngineUtils.h"
#include "GameMode/T2GameModeBase.h"
#include "T2PlayGameState.h"

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
            // Reset all state when returning to title
            ResetAllUIState();
            
            // Check if we need to auto-connect to host
            // In PIE, client instances should try to connect to the host
            UWorld* World = GetWorld();
            if (World)
            {
                ENetMode NetMode = World->GetNetMode();
                
                // If standalone (not yet connected), try to connect after a short delay
                // This gives the host time to start listening
                if (NetMode == NM_Standalone)
                {
                    // Check PIE instance index - first instance (0) becomes host, others join
                    int32 PIEInstance = GPlayInEditorID;
                    
                    if (PIEInstance > 0)
                    {
                        // Not the first instance - try to join the host
                        UE_LOG(LogTemp, Warning, TEXT("PIE Instance %d - Will auto-join host in 1 second"), PIEInstance);
                        
                        FTimerHandle JoinTimer;
                        World->GetTimerManager().SetTimer(JoinTimer, [this]()
                        {
                            UE_LOG(LogTemp, Warning, TEXT("Auto-joining host at 127.0.0.1"));
                            ClientTravel(TEXT("127.0.0.1"), ETravelType::TRAVEL_Absolute);
                        }, 1.0f, false);
                    }
                    else
                    {
                        // First instance - show title UI (host will be started by GameMode)
                        UE_LOG(LogTemp, Warning, TEXT("PIE Instance 0 - This is the host"));
                        ShowTitleUI();
                    }
                }
                else
                {
                    // Already connected as client or listen server
                    ShowTitleUI();
                }
            }
            else
            {
                ShowTitleUI();
            }
        }

        BindRoleChangedDelegate();
    }
}

void AT2BaseController::ResetAllUIState()
{
    UE_LOG(LogTemp, Warning, TEXT("ResetAllUIState: Cleaning up all UI"));

    // Remove and null all widget instances
    if (TitleWidgetInstance)
    {
        TitleWidgetInstance->RemoveFromParent();
        TitleWidgetInstance = nullptr;
    }

    if (LobbyWidgetInstance)
    {
        LobbyWidgetInstance->RemoveFromParent();
        LobbyWidgetInstance = nullptr;
    }

    if (PersonalResultInstance)
    {
        PersonalResultInstance->RemoveFromParent();
        PersonalResultInstance = nullptr;
    }

    if (SettingsMenuInstance)
    {
        SettingsMenuInstance->RemoveFromParent();
        SettingsMenuInstance = nullptr;
    }

    if (KillerHUDInstance)
    {
        KillerHUDInstance->RemoveFromParent();
        KillerHUDInstance = nullptr;
    }

    if (SurvivorHUDInstance)
    {
        SurvivorHUDInstance->RemoveFromParent();
        SurvivorHUDInstance = nullptr;
    }

    if (InteractWidgetClassInstance)
    {
        InteractWidgetClassInstance->RemoveFromParent();
        InteractWidgetClassInstance = nullptr;
    }

    // Reset state flags
    bIsSpectating = false;
    bWasEscaped = false;
    bHUDInitialized = false;
    bIsSettingsMenuOpen = false;
    bInteractUIActive = false;
    CurrentDisplayedRole = EPlayerRole::None;
}

void AT2BaseController::ShowTitleUI()
{
    if (!IsLocalPlayerController()) return;

    UE_LOG(LogTemp, Warning, TEXT("ShowTitleUI: Started"));

    if (!TitleWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("TitleWidgetClass is not set!"));
        return;
    }

    // Always create fresh widget
    TitleWidgetInstance = CreateWidget<UUserWidget>(this, TitleWidgetClass);

    if (TitleWidgetInstance)
    {
        TitleWidgetInstance->AddToViewport();

        bShowMouseCursor = true;
        FInputModeUIOnly InputMode;
        SetInputMode(InputMode);

        TArray<AActor*> FoundCameras;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("TitleCamera"), FoundCameras);

        if (FoundCameras.Num() > 0)
        {
            SetViewTargetWithBlend(FoundCameras[0], 0.0f);
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

    TArray<AActor*> FoundCameras;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("SelectCamera"), FoundCameras);

    if (FoundCameras.Num() > 0)
    {
        SetViewTargetWithBlend(FoundCameras[0], 1.0f);
    }

    ShowLobbyUI();
}

void AT2BaseController::OnTitleStartButtonPressed()
{
    if (!IsLocalPlayerController()) return;

    UE_LOG(LogTemp, Warning, TEXT("OnTitleStartButtonPressed Called"));

    // Simply transition to lobby
    // GameMode already started listen server in BeginPlay
    // Other clients will auto-join via BeginPlay timer
    TransitionToLobby();
}

void AT2BaseController::ShowLobbyUI()
{
    if (!IsLocalPlayerController()) return;

    if (!LobbyWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("LobbyWidgetClass is not set!"));
        return;
    }

    // Always create fresh widget
    LobbyWidgetInstance = CreateWidget<UUserWidget>(this, LobbyWidgetClass);

    if (LobbyWidgetInstance)
    {
        LobbyWidgetInstance->AddToViewport();

        bShowMouseCursor = true;
        FInputModeUIOnly InputMode;
        SetInputMode(InputMode);
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

    BindRoleChangedDelegate();

    if (AT2PlayerState* PS = GetPlayerState<AT2PlayerState>())
    {
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
            if (PS->PlayerRole != EPlayerRole::None)
            {
                HideAllHUD();

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

    if (bDelegateBound && BoundPlayerState == PS)
    {
        return;
    }

    if (bDelegateBound && BoundPlayerState && BoundPlayerState != PS)
    {
        BoundPlayerState->OnPlayerRoleChanged.RemoveDynamic(this, &AT2BaseController::OnPlayerRoleChanged);
        bDelegateBound = false;
    }

    PS->OnPlayerRoleChanged.AddDynamic(this, &AT2BaseController::OnPlayerRoleChanged);
    BoundPlayerState = PS;
    bDelegateBound = true;
}

void AT2BaseController::SetupInputComponent()
{
    Super::SetupInputComponent();

    InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AT2BaseController::ToggleSettingsMenu);
}

void AT2BaseController::UpdateHUDForRole(EPlayerRole NewRole)
{
    if (!IsLocalPlayerController()) return;

    if (NewRole == EPlayerRole::None) return;

    if (AT2PlayerState* PS = GetPlayerState<AT2PlayerState>())
    {
        if (PS->PlayerRole != NewRole) return;
    }

    if (bHUDInitialized && CurrentDisplayedRole == NewRole) return;

    CurrentDisplayedRole = NewRole;
    bHUDInitialized = true;

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
    if (!KillerHUDWidgetClass) return;

    if (!KillerHUDInstance)
    {
        KillerHUDInstance = CreateWidget<UUW_KillerHUD>(this, KillerHUDWidgetClass);
    }

    if (KillerHUDInstance)
    {
        KillerHUDInstance->AddToViewport();
    }
}

void AT2BaseController::ShowSurvivorHUD()
{
    if (!SurvivorHUDWidgetClass) return;

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

    AT2PlayerState* MyPS = GetPlayerState<AT2PlayerState>();
    if (!MyPS) return;

    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    if (MyPawn->GetClass()->GetName().Contains(TEXT("DefaultPawn"))) return;

    if (MyPS->PlayerRole != NewRole) return;

    UpdateHUDForRole(NewRole);
}

void AT2BaseController::ToggleSettingsMenu()
{
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
    if (!SettingsMenuWidgetClass) return;

    if (!SettingsMenuInstance)
    {
        SettingsMenuInstance = CreateWidget<UUserWidget>(this, SettingsMenuWidgetClass);
    }

    if (SettingsMenuInstance)
    {
        SettingsMenuInstance->AddToViewport(100);
        bIsSettingsMenuOpen = true;

        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(SettingsMenuInstance->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        SetInputMode(InputMode);
        bShowMouseCursor = true;
    }
}

void AT2BaseController::ServerRequestStartGame_Implementation()
{
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
}

void AT2BaseController::Client_OnPlayerCountChanged_Implementation(int32 NewCount)
{
    UE_LOG(LogTemp, Warning, TEXT("Client_OnPlayerCountChanged: %d players in lobby"), NewCount);
    
    // Update lobby UI if it exists
    // The widget can bind to this or we can expose a function
    // For now, just log - the widget can poll GetCurrentPlayerCount()
}

void AT2BaseController::CloseSettingsMenu()
{
    if (SettingsMenuInstance && SettingsMenuInstance->IsInViewport())
    {
        SettingsMenuInstance->RemoveFromParent();
    }

    bIsSettingsMenuOpen = false;

    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = false;
}

void AT2BaseController::RequestLeaveGame()
{
    UE_LOG(LogTemp, Warning, TEXT("RequestLeaveGame: NetMode = %d"), (int32)GetNetMode());

    // Check if we're in gameplay map - if so, ignore this call
    // The server will handle returning to title via ReturnToTitle()
    FString MapName = GetWorld()->GetMapName();
    MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
    
    if (!MapName.Contains(TEXT("title")))
    {
        UE_LOG(LogTemp, Warning, TEXT("RequestLeaveGame: Ignored - not in title map. Server will handle return."));
        return;
    }

    // Only allow leaving from title screen (e.g., settings menu -> quit)
    if (UWorld* World = GetWorld())
    {
        if (GetNetMode() == NM_Client)
        {
            UE_LOG(LogTemp, Warning, TEXT("Client: Disconnecting and returning to title"));
            ClientTravel(TEXT("/Game/Team02/Blueprint/map/title"), ETravelType::TRAVEL_Absolute);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Host/Standalone: Loading title"));
            UGameplayStatics::OpenLevel(World, TEXT("/Game/Team02/Blueprint/map/title"));
        }
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

    if (IsValid(ProgressImage))
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

void AT2BaseController::Client_ShowPersonalResult_Implementation(bool bEscaped)
{
    if (!IsLocalPlayerController()) return;

    UE_LOG(LogTemp, Warning, TEXT("Client_ShowPersonalResult: bEscaped = %s"), bEscaped ? TEXT("TRUE") : TEXT("FALSE"));

    bWasEscaped = bEscaped;

    HideAllHUD();

    if (!PersonalResultWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("PersonalResultWidgetClass is not set!"));
        return;
    }

    PersonalResultInstance = CreateWidget<UUW_PersonalResult>(this, PersonalResultWidgetClass);

    if (PersonalResultInstance)
    {
        PersonalResultInstance->SetResult(bEscaped);
        PersonalResultInstance->AddToViewport(50);

        FInputModeGameAndUI InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        SetInputMode(InputMode);
        bShowMouseCursor = true;

        UE_LOG(LogTemp, Warning, TEXT("PersonalResultWidget displayed!"));
    }
}

void AT2BaseController::Client_ShowMatchResult_Implementation(bool bSurvivorWin)
{
    if (!IsLocalPlayerController()) return;

    UE_LOG(LogTemp, Warning, TEXT("Client_ShowMatchResult: bSurvivorWin = %s"), bSurvivorWin ? TEXT("TRUE") : TEXT("FALSE"));

    // If spectating, stop spectating
    if (SpawnedSpectatorPawn)
    {
        SpawnedSpectatorPawn = nullptr;
    }
    bIsSpectating = false;

    // If result widget already exists, add match result
    if (PersonalResultInstance && PersonalResultInstance->IsInViewport())
    {
        PersonalResultInstance->ShowMatchResult(bSurvivorWin);
    }
    else
    {
        if (!PersonalResultWidgetClass) return;

        PersonalResultInstance = CreateWidget<UUW_PersonalResult>(this, PersonalResultWidgetClass);
        if (PersonalResultInstance)
        {
            PersonalResultInstance->SetResult(bWasEscaped);
            PersonalResultInstance->ShowMatchResult(bSurvivorWin);
            PersonalResultInstance->AddToViewport(50);

            FInputModeUIOnly InputMode;
            SetInputMode(InputMode);
            bShowMouseCursor = true;
        }
    }

    HideAllHUD();
}

// Called from UI button (client side)
void AT2BaseController::StartSpectating()
{
    if (!IsLocalPlayerController()) return;
    
    UE_LOG(LogTemp, Warning, TEXT("=== StartSpectating Called (Client) ==="));
    
    // Remove result widget
    if (PersonalResultInstance && PersonalResultInstance->IsInViewport())
    {
        PersonalResultInstance->RemoveFromParent();
        PersonalResultInstance = nullptr;
    }

    // Request server to spawn and possess spectator pawn
    Server_RequestSpectate();
}

// Server RPC - spawn spectator pawn and possess
void AT2BaseController::Server_RequestSpectate_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Server_RequestSpectate Called ==="));

    if (!SpectatorPawnClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Server_RequestSpectate: SpectatorPawnClass is not set!"));
        return;
    }

    // Find alive survivor to spectate
    APawn* Target = GetAliveTarget();
    if (!Target)
    {
        UE_LOG(LogTemp, Warning, TEXT("Server_RequestSpectate: No alive target found"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Server_RequestSpectate: Target = %s at %s"), 
        *Target->GetName(), *Target->GetActorLocation().ToString());

    // Spawn spectator pawn at target location
    FVector SpawnLoc = Target->GetActorLocation();
    FRotator SpawnRot = FRotator::ZeroRotator;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AAT2SpectatorPawn* NewSpectatorPawn = GetWorld()->SpawnActor<AAT2SpectatorPawn>(
        SpectatorPawnClass,
        SpawnLoc,
        SpawnRot,
        SpawnParams
    );

    if (NewSpectatorPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("Server_RequestSpectate: SpectatorPawn spawned"));

        // Set spectate target BEFORE possessing
        NewSpectatorPawn->SetSpectateTarget(Target);

        // Possess on server (this will replicate to client)
        Possess(NewSpectatorPawn);

        // Store reference
        SpawnedSpectatorPawn = NewSpectatorPawn;

        UE_LOG(LogTemp, Warning, TEXT("Server_RequestSpectate: Possessed SpectatorPawn"));

        // Notify client
        Client_OnSpectateStarted();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Server_RequestSpectate: Failed to spawn SpectatorPawn"));
    }
}

// Client RPC - spectating started
void AT2BaseController::Client_OnSpectateStarted_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Client_OnSpectateStarted ==="));

    bIsSpectating = true;

    // Set input mode for spectating - game only, no mouse cursor
    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = false;
}

void AT2BaseController::Client_ReturnToTitle_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Client_ReturnToTitle Called ==="));

    if (!IsLocalPlayerController()) return;

    // Clean up UI
    ResetAllUIState();

    // Disconnect from server and load title locally
    ClientTravel(TEXT("/Game/Team02/Blueprint/map/title"), ETravelType::TRAVEL_Absolute);
}

APawn* AT2BaseController::GetAliveTarget()
{
    UE_LOG(LogTemp, Warning, TEXT("=== GetAliveTarget Called ==="));
    
    // Find alive survivor (not dead, not escaped, not killer)
    TArray<AActor*> FoundCharacters;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AT2PlayerCharacter::StaticClass(), FoundCharacters);

    UE_LOG(LogTemp, Warning, TEXT("GetAliveTarget: Found %d AT2PlayerCharacter actors"), FoundCharacters.Num());

    for (AActor* Actor : FoundCharacters)
    {
        AT2PlayerCharacter* PlayerChar = Cast<AT2PlayerCharacter>(Actor);
        if (!PlayerChar) continue;

        // Skip hidden actors
        if (PlayerChar->IsHidden())
        {
            UE_LOG(LogTemp, Warning, TEXT("GetAliveTarget: %s is hidden, skipping"), *PlayerChar->GetName());
            continue;
        }

        APlayerState* PS = PlayerChar->GetPlayerState();
        if (!PS) continue;

        // Check if this is a Survivor (not a Killer)
        ASurvivorPlayerState* SurvivorPS = Cast<ASurvivorPlayerState>(PS);
        if (!SurvivorPS)
        {
            UE_LOG(LogTemp, Warning, TEXT("GetAliveTarget: %s is not a Survivor, skipping"), *PlayerChar->GetName());
            continue;
        }

        UE_LOG(LogTemp, Warning, TEXT("GetAliveTarget: Checking %s - bIsDead=%s, bIsEscaped=%s"),
            *PlayerChar->GetName(),
            SurvivorPS->bIsDead ? TEXT("TRUE") : TEXT("FALSE"),
            SurvivorPS->bIsEscaped ? TEXT("TRUE") : TEXT("FALSE"));

        if (!SurvivorPS->bIsDead && !SurvivorPS->bIsEscaped)
        {
            UE_LOG(LogTemp, Warning, TEXT("GetAliveTarget: Found alive survivor %s!"), *PlayerChar->GetName());
            return PlayerChar;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("GetAliveTarget: No alive survivor found!"));
    return nullptr;
}
