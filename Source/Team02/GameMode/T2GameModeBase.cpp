#include "GameMode/T2GameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

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

void AT2GameModeBase::TransitionToRoleSelect()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

    if (PC)
    {
        AActor* SelectCamera = FindCameraByTag(FName("SelectCamera"));
        if (SelectCamera)
        {
            PC->SetViewTargetWithBlend(SelectCamera, CameraBlendTime);
        }
    }

    SwitchWidget(RoleSelectWidgetClass);
}

void AT2GameModeBase::StartGameAsKiller()
{
    StartGameWithRole(EPlayerRole::Killer);
}

void AT2GameModeBase::StartGameAsSurvivor()
{
    StartGameWithRole(EPlayerRole::Survivor);
}

void AT2GameModeBase::OnPlayerDead(AT2PlayerCharacter* DeadCharacter)
{
    AT2PlayerController* PC = Cast<AT2PlayerController>(DeadCharacter->GetController());
    if (IsValid(PC) == false)
    {
        return;
    }

    ////관전자 상태로 전환
    //PC->StartSpectate(Target);

    ////관전 대상 선택
    //ACharacter* AliverPlayer = FindAlivePlayerExcept(DeadCharacter);

    //if (AlivePlayer)
    //{
    //    PC->SetViewTargetWithBlend(AlivePlayer, 0.5f);
    //}
}

void AT2GameModeBase::StartGameWithRole(EPlayerRole InRole)
{

    UT2GameInstance* GI = Cast<UT2GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    if (GI)
    {
        GI->SelectedRole = InRole;
    }

  
    UGameplayStatics::OpenLevel(GetWorld(), GamePlayMapName);
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