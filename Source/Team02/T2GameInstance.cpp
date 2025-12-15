#include "T2GameInstance.h"
#include "Kismet/GameplayStatics.h"

void UT2GameInstance::HostGame(const FString& MapName)
{
    UWorld* World = GetWorld();
    if (World)
    {

        FString URL = FString::Printf(TEXT("/Game/Library_Pack/Maps/%s? listen"), *MapName);
        World->ServerTravel(URL);
        UE_LOG(LogTemp, Warning, TEXT("HostGame - ServerTravel to:  %s"), *URL);
    }
}

void UT2GameInstance::JoinGame(const FString& IPAddress)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        PC->ClientTravel(IPAddress, ETravelType::TRAVEL_Absolute);
        UE_LOG(LogTemp, Warning, TEXT("JoinGame - Connecting to: %s"), *IPAddress);
    }
}