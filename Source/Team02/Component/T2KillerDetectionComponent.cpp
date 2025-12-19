// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/T2KillerDetectionComponent.h"

#include "Character/PlayerCharacter/T2PlayerCharacter.h"
#include "PlayerState/Player/SurvivorPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

UT2KillerDetectionComponent::UT2KillerDetectionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; 
}

void UT2KillerDetectionComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UT2KillerDetectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
    {
        return;
    }

    UpdateDetectedSurvivors();
}

void UT2KillerDetectionComponent::UpdateDetectedSurvivors()
{
    TArray<AActor*> FoundSurvivors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AT2PlayerCharacter::StaticClass(), FoundSurvivors);

    // 현재 감지된 생존자 목록
    TSet<AT2PlayerCharacter*> CurrentlyDetected;

    for (AActor* Actor : FoundSurvivors)
    {
        AT2PlayerCharacter* Survivor = Cast<AT2PlayerCharacter>(Actor);
        if (!Survivor) continue;

        ASurvivorPlayerState* SurvivorPS = Survivor->GetPlayerState<ASurvivorPlayerState>();
        if (!SurvivorPS) continue;

        // 비전 디버프가 있고 죽지 않은 생존자만 감지
        if (SurvivorPS->IsDetectedByKiller() && !SurvivorPS->bIsDead)
        {
            CurrentlyDetected.Add(Survivor);

            // 아직 머티리얼이 적용되지 않은 경우
            if (!DetectedSurvivorMaterials.Contains(Survivor))
            {
                ApplyAuraMaterial(Survivor);
            }
        }
    }

    // 더 이상 감지되지 않는 생존자의 머티리얼 제거
    TArray<AT2PlayerCharacter*> ToRemove;
    for (auto& Pair : DetectedSurvivorMaterials)
    {
        if (!CurrentlyDetected.Contains(Pair.Key))
        {
            ToRemove.Add(Pair.Key);
        }
    }

    for (AT2PlayerCharacter* Survivor : ToRemove)
    {
        RemoveAuraMaterial(Survivor);
    }
}

void UT2KillerDetectionComponent::ApplyAuraMaterial(AT2PlayerCharacter* Survivor)
{
    if (!Survivor || !AuraMaterialBase) return;

    USkeletalMeshComponent* Mesh = Survivor->GetMesh();
    if (!Mesh) return;

    // 다이나믹 머티리얼 인스턴스 생성
    UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(AuraMaterialBase, this);
    if (!DynMaterial) return;

    // 파라미터 설정 (머티리얼에 이 파라미터들이 있어야 함)
    DynMaterial->SetVectorParameterValue(FName("AuraColor"), AuraColor);
    DynMaterial->SetScalarParameterValue(FName("AuraIntensity"), AuraIntensity);

    // 오버레이 머티리얼로 설정 (기존 머티리얼 위에 표시)
    Mesh->SetOverlayMaterial(DynMaterial);

    DetectedSurvivorMaterials.Add(Survivor, DynMaterial);

    UE_LOG(LogTemp, Warning, TEXT("Aura Material Applied to %s"), *Survivor->GetName());
}

void UT2KillerDetectionComponent::RemoveAuraMaterial(AT2PlayerCharacter* Survivor)
{
    if (!Survivor) return;

    USkeletalMeshComponent* Mesh = Survivor->GetMesh();
    if (Mesh)
    {
        Mesh->SetOverlayMaterial(nullptr);
    }

    DetectedSurvivorMaterials.Remove(Survivor);

    UE_LOG(LogTemp, Warning, TEXT("Aura Material Removed from %s"), *Survivor->GetName());
}