#include "GameMode/T2GameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Controller/T2BaseController.h"

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

    // ServerTravel로 모든 클라이언트와 함께 이동
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
    // 관전 기능 - 나중에 구현
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