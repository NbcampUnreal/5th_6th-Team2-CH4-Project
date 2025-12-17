#include "T2PlayGameMod.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "PlayerState/T2PlayerState.h"

AT2PlayGameMod::AT2PlayGameMod()
{
    
    
}

void AT2PlayGameMod::BeginPlay()
{
    Super::BeginPlay();
}

APlayerController* AT2PlayGameMod::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{

    EPlayerRole AssignedRole = AssignRoleForNewPlayer(Options);


    TSubclassOf<APlayerController> ControllerClassToUse = nullptr;

    if (AssignedRole == EPlayerRole::Killer && KillerControllerClass)
    {
        ControllerClassToUse = KillerControllerClass;
    }
    else if (AssignedRole == EPlayerRole::Survivor && SurvivorControllerClass)
    {
        ControllerClassToUse = SurvivorControllerClass;
    }
    else
    {
        ControllerClassToUse = PlayerControllerClass;
    }


    TSubclassOf<APlayerController> OriginalClass = PlayerControllerClass;
    PlayerControllerClass = ControllerClassToUse;

    APlayerController* NewController = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);


    PlayerControllerClass = OriginalClass;


    if (NewController)
    {
        PlayerRoles.Add(NewController, AssignedRole);
    }

    return NewController;
}

EPlayerRole AT2PlayGameMod::AssignRoleForNewPlayer(const FString& Options)
{

    UT2GameInstance* GI = Cast<UT2GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));


    FString RoleOption = UGameplayStatics::ParseOption(Options, TEXT("Role"));

    if (!RoleOption.IsEmpty())
    {
        if (RoleOption == TEXT("Killer") && !bKillerTaken)
        {
            bKillerTaken = true;
            return EPlayerRole::Killer;
        }
        else if (RoleOption == TEXT("Survivor") && !bSurvivorTaken)
        {
            bSurvivorTaken = true;
            return EPlayerRole::Survivor;
        }
    }


    if (GI && GI->SelectedRole != EPlayerRole::None)
    {
        EPlayerRole WantedRole = GI->SelectedRole;

        if (WantedRole == EPlayerRole::Killer && !bKillerTaken)
        {
            bKillerTaken = true;
            return EPlayerRole::Killer;
        }
        else if (WantedRole == EPlayerRole::Survivor && !bSurvivorTaken)
        {
            bSurvivorTaken = true;
            return EPlayerRole::Survivor;
        }
    }

  
    if (!bKillerTaken)
    {
        bKillerTaken = true;
        return EPlayerRole::Killer;
    }
    else if (!bSurvivorTaken)
    {
        bSurvivorTaken = true;
        return EPlayerRole::Survivor;
    }

    return EPlayerRole::None;
}

void AT2PlayGameMod::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (!NewPlayer) return;

    if (AT2PlayerState* PS = NewPlayer->GetPlayerState<AT2PlayerState>())
    {
        EPlayerRole* FoundRole = PlayerRoles.Find(NewPlayer);
        if (FoundRole)
        {
            PS->SetPlayerRole(*FoundRole);
        }
    }

    FInputModeGameOnly InputMode;
    NewPlayer->SetInputMode(InputMode);
    NewPlayer->bShowMouseCursor = false;
}

void AT2PlayGameMod::Logout(AController* Exiting)
{
    APlayerController* PC = Cast<APlayerController>(Exiting);
    if (PC && PlayerRoles.Contains(PC))
    {
        EPlayerRole ExitingRole = PlayerRoles[PC];

        if (ExitingRole == EPlayerRole::Killer)
        {
            bKillerTaken = false;
        }
        else if (ExitingRole == EPlayerRole::Survivor)
        {
            bSurvivorTaken = false;
        }

        PlayerRoles.Remove(PC);
    }

    Super::Logout(Exiting);
}

EPlayerRole AT2PlayGameMod::GetPlayerRoleFromMap(AController* Player)
{
    APlayerController* PC = Cast<APlayerController>(Player);
    if (PC && PlayerRoles.Contains(PC))
    {
        return PlayerRoles[PC];
    }
    return EPlayerRole::None;
}

UClass* AT2PlayGameMod::GetDefaultPawnClassForController_Implementation(AController* InController)
{
    EPlayerRole FoundRole = GetPlayerRoleFromMap(InController);

    if (FoundRole == EPlayerRole::Killer && KillerClass)
    {
        return KillerClass;
    }
    else if (FoundRole == EPlayerRole::Survivor && SurvivorClass)
    {
        return SurvivorClass;
    }

    return Super::GetDefaultPawnClassForController_Implementation(InController);
}

AActor* AT2PlayGameMod::ChoosePlayerStart_Implementation(AController* Player)
{
    EPlayerRole FoundRole = GetPlayerRoleFromMap(Player);
    FName SpawnTag = (FoundRole == EPlayerRole::Killer) ? FName("KillerSpawn") : FName("SurvivorSpawn");

    TArray<AActor*> SpawnPoints;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), SpawnTag, SpawnPoints);

    if (SpawnPoints.Num() > 0)
    {
        return SpawnPoints[0];
    }

    return Super::ChoosePlayerStart_Implementation(Player);
}