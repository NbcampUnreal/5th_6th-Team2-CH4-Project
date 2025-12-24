#include "GameMode/T2GameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Controller/T2BaseController.h"
#include "NavigationSystem.h"
#include "GameFramework/PlayerStart.h"
#include "Gimmick/Portal/PortalActor.h"

AT2GameModeBase::AT2GameModeBase()
{
}

void AT2GameModeBase::BeginPlay()
{
    Super::BeginPlay();

      APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

    if (PC)
    {
        PC->bShowMouseCursor = true;
        FInputModeUIOnly InputMode;
        PC->SetInputMode(InputMode);

        AActor* TitleCamera = FindCameraByTag(FName("TitleCamera"));
        if (TitleCamera)
        {
            PC->SetViewTargetWithBlend(TitleCamera, 0.0f);
        }
    }

    SwitchWidget(TitleWidgetClass);
}

void AT2GameModeBase::TransitionToLobby()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

    if (PC)
    {
        AActor* LobbyCamera = FindCameraByTag(FName("SelectCamera"));
        if (LobbyCamera)
        {
            PC->SetViewTargetWithBlend(LobbyCamera, CameraBlendTime);
        }
    }

    SwitchWidget(LobbyWidgetClass);
}

int32 AT2GameModeBase::GetCurrentPlayerCount()
{
    return GetNumPlayers();
}

bool AT2GameModeBase::CanStartGame()
{
    return GetNumPlayers() >= RequiredPlayers;
}

void AT2GameModeBase::TryStartGame()
{
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("TryStartGame: Not server! "));
        return;
    }

    if (!CanStartGame())
    {
        UE_LOG(LogTemp, Warning, TEXT("TryStartGame: Not enough players (%d/%d)"),
            GetNumPlayers(), RequiredPlayers);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("TryStartGame: Starting with %d players! "), GetNumPlayers());

    // ServerTravel�� ��� Ŭ���̾�Ʈ�� �Բ� �̵�
    GetWorld()->ServerTravel(FString::Printf(TEXT("/Game/Library_Pack/Maps/%s? listen"), *GamePlayMapName.ToString()));
}

void AT2GameModeBase::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    UE_LOG(LogTemp, Warning, TEXT("Lobby PostLogin: Player joined.  Total: %d"), GetNumPlayers());
}

void AT2GameModeBase::Logout(AController* Exiting)
{
    Super::Logout(Exiting);
    UE_LOG(LogTemp, Warning, TEXT("Lobby Logout: Player left.  Total: %d"), GetNumPlayers() - 1);
}

void AT2GameModeBase::OnPlayerDead(AT2PlayerCharacter* DeadCharacter)
{
    AT2BaseController* PC = Cast<AT2BaseController>(DeadCharacter->GetController());
    if (PC == nullptr)
    {
        return;
    }
    // ���� ��� - ���߿� ����
}

AActor* AT2GameModeBase::FindCameraByTag(FName Tag)
{
    TArray<AActor*> FoundCameras;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), Tag, FoundCameras);

    if (FoundCameras.Num() > 0)
    {
        return FoundCameras[0];
    }
    return nullptr;
}

void AT2GameModeBase::SwitchWidget(TSubclassOf<UUserWidget> NewWidgetClass)
{
    if (CurrentWidget)
    {
        CurrentWidget->RemoveFromParent();
        CurrentWidget = nullptr;
    }

    if (NewWidgetClass)
    {
        CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), NewWidgetClass);
        if (CurrentWidget)
        {
            CurrentWidget->AddToViewport();
        }
    }
}

void AT2GameModeBase::OnKeyCollected(int32 CurrentTotalKeys)
{
    UE_LOG(LogTemp, Warning, TEXT(" OnKeyCollected called! Current Keys: %d / Required: %d"), 
       CurrentTotalKeys, KeysRequiredForPortal);

    if (CurrentTotalKeys >= KeysRequiredForPortal)
    {
        UE_LOG(LogTemp, Warning, TEXT(" Keys requirement met! Spawning portal..."));
        SpawnPortalAtRandomLocation();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT(" Need %d more keys for portal"), 
            KeysRequiredForPortal - CurrentTotalKeys);
    }
}

void AT2GameModeBase::SpawnPortalAtRandomLocation()
{
     if (!HasAuthority())
    {
        UE_LOG(LogTemp, Error, TEXT(" SpawnPortal: Not Authority!"));
        return;
    }

    if (!PortalClass)
    {
        UE_LOG(LogTemp, Error, TEXT(" PortalClass is not set in GameMode!"));
        return;
    }

    FVector SpawnLocation = FVector::ZeroVector;
    bool bFoundLocation = false;

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (NavSys)
    {
        FNavLocation RandomLocation;
        if (NavSys->GetRandomReachablePointInRadius(FVector::ZeroVector, 5000.f, RandomLocation))
        {
            SpawnLocation = RandomLocation.Location + FVector(0, 0, 100); // 공중에 띄우기
            bFoundLocation = true;
            UE_LOG(LogTemp, Warning, TEXT(" Portal spawn location found via NavMesh: %s"), 
                *SpawnLocation.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT(" NavMesh random point failed, trying PlayerStart..."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT(" NavigationSystem not found, trying PlayerStart..."));
    }
    

    // 포탈 스폰
    if (bFoundLocation)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        APortalActor* Portal = GetWorld()->SpawnActor<APortalActor>(
            PortalClass, 
            SpawnLocation, 
            FRotator::ZeroRotator, 
            SpawnParams
        );
        
        if (Portal)
        {
            Portal->PortalTimeLimit = PortalDuration;
            Portal->ActivatePortal();
            UE_LOG(LogTemp, Warning, TEXT(" Portal spawned successfully at %s! Duration: %.0f seconds"), 
                *SpawnLocation.ToString(), PortalDuration);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT(" Failed to spawn Portal actor!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Could not find valid spawn location for portal!"));
    }
}
