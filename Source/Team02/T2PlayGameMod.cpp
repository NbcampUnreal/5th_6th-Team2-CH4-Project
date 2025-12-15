#include "T2PlayGameMod.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

AT2PlayGameMod::AT2PlayGameMod()
{
    // ★ 기본 Pawn 클래스를 nullptr로 두지 말고, 하나 지정 ★
    // DefaultPawnClass = nullptr;  // 이거 삭제하거나 주석처리
}

void AT2PlayGameMod::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("=== T2PlayGameMod BeginPlay ==="));
}

void AT2PlayGameMod::PostLogin(APlayerController* NewPlayer)
{
    UE_LOG(LogTemp, Warning, TEXT("=== PostLogin START ==="));

    if (!NewPlayer)
    {
        UE_LOG(LogTemp, Error, TEXT("NewPlayer is NULL! "));
        return;
    }

    // ★ 역할 배정을 Super::PostLogin 전에 해야 함!  ★
    EPlayerRole AssignedRole = AssignRoleToPlayer(NewPlayer);
    PlayerRoles.Add(NewPlayer, AssignedRole);

    UE_LOG(LogTemp, Warning, TEXT("Role Assigned: %d (1=Killer, 2=Survivor)"), (int32)AssignedRole);

    // ★ 이제 Super 호출 (여기서 Pawn 스폰됨) ★
    Super::PostLogin(NewPlayer);

    // 입력 모드 설정
    FInputModeGameOnly InputMode;
    NewPlayer->SetInputMode(InputMode);
    NewPlayer->bShowMouseCursor = false;

    UE_LOG(LogTemp, Warning, TEXT("=== PostLogin END ==="));

    // 스폰된 Pawn 확인
    if (NewPlayer->GetPawn())
    {
        UE_LOG(LogTemp, Warning, TEXT("Pawn Spawned: %s"), *NewPlayer->GetPawn()->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("No Pawn spawned!"));
    }
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
        UE_LOG(LogTemp, Warning, TEXT("Player Left.  Role %d released. "), (int32)ExitingRole);
    }

    Super::Logout(Exiting);
}

EPlayerRole AT2PlayGameMod::AssignRoleToPlayer(APlayerController* Player)
{
    // 호스트는 GameInstance에서 선택한 역할 사용
    if (Player && Player->IsLocalController())
    {
        UT2GameInstance* GI = Cast<UT2GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
        if (GI && GI->SelectedRole != EPlayerRole::None)
        {
            EPlayerRole WantedRole = GI->SelectedRole;

            if (WantedRole == EPlayerRole::Killer && !bKillerTaken)
            {
                bKillerTaken = true;
                UE_LOG(LogTemp, Warning, TEXT("Host -> Killer"));
                return EPlayerRole::Killer;
            }
            else if (WantedRole == EPlayerRole::Survivor && !bSurvivorTaken)
            {
                bSurvivorTaken = true;
                UE_LOG(LogTemp, Warning, TEXT("Host -> Survivor"));
                return EPlayerRole::Survivor;
            }
        }
    }

    // 클라이언트는 남은 역할 자동 배정
    if (!bKillerTaken)
    {
        bKillerTaken = true;
        UE_LOG(LogTemp, Warning, TEXT("Auto -> Killer"));
        return EPlayerRole::Killer;
    }
    else if (!bSurvivorTaken)
    {
        bSurvivorTaken = true;
        UE_LOG(LogTemp, Warning, TEXT("Auto -> Survivor"));
        return EPlayerRole::Survivor;
    }

    return EPlayerRole::None;
}

EPlayerRole AT2PlayGameMod::GetPlayerRoleFromMap(AController* Player)
{
    APlayerController* PC = Cast<APlayerController>(Player);
    if (PC && PlayerRoles.Contains(PC))
    {
        return PlayerRoles[PC];
    }

    // ★ 역할이 없으면 호스트의 GameInstance에서 가져오기 ★
    if (Player && Player->IsLocalController())
    {
        UT2GameInstance* GI = Cast<UT2GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
        if (GI && GI->SelectedRole != EPlayerRole::None)
        {
            return GI->SelectedRole;
        }
    }

    return EPlayerRole::None;
}

UClass* AT2PlayGameMod::GetDefaultPawnClassForController_Implementation(AController* InController)
{
    UE_LOG(LogTemp, Warning, TEXT("=== GetDefaultPawnClass ==="));

    EPlayerRole FoundRole = GetPlayerRoleFromMap(InController);
    UE_LOG(LogTemp, Warning, TEXT("Role: %d"), (int32)FoundRole);

    if (FoundRole == EPlayerRole::Killer)
    {
        UE_LOG(LogTemp, Warning, TEXT("KillerClass: %s"), KillerClass ? *KillerClass->GetName() : TEXT("NULL"));
        if (KillerClass)
        {
            return KillerClass;
        }
    }
    else if (FoundRole == EPlayerRole::Survivor)
    {
        UE_LOG(LogTemp, Warning, TEXT("SurvivorClass: %s"), SurvivorClass ? *SurvivorClass->GetName() : TEXT("NULL"));
        if (SurvivorClass)
        {
            return SurvivorClass;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Using Super class"));
    return Super::GetDefaultPawnClassForController_Implementation(InController);
}

AActor* AT2PlayGameMod::ChoosePlayerStart_Implementation(AController* Player)
{
    EPlayerRole FoundRole = GetPlayerRoleFromMap(Player);
    FName SpawnTag = (FoundRole == EPlayerRole::Killer) ? FName("KillerSpawn") : FName("SurvivorSpawn");

    TArray<AActor*> SpawnPoints;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), SpawnTag, SpawnPoints);

    UE_LOG(LogTemp, Warning, TEXT("SpawnTag: %s, Found:  %d"), *SpawnTag.ToString(), SpawnPoints.Num());

    if (SpawnPoints.Num() > 0)
    {
        return SpawnPoints[0];
    }

    return Super::ChoosePlayerStart_Implementation(Player);
}