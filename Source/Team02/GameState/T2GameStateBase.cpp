#include "GameState/T2GameStateBase.h"
#include "Net/UnrealNetwork.h"

void AT2GameStateBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AT2GameStateBase, Countdown);
}















//void AT2GameStateBase::AddKeyCount(int32 Amount)
//{
//	TotalKeyCount += Amount;
//	OnRep_KeyCount();
//
//	UE_LOG(LogTemp, Warning, TEXT("TotalKeyCount : %d"), TotalKeyCount);
//}
//
//void AT2GameStateBase::OnRep_KeyCount()
//{
//	//UI ¹Ý¿µ
//}