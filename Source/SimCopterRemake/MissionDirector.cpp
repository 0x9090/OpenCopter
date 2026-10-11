#include "MissionDirector.h"

#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "HelicopterPawn.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
const TCHAR* MissionTypeName(ESimCopterMissionType Type)
{
    switch (Type)
    {
    case ESimCopterMissionType::General: return TEXT("General assistance");
    case ESimCopterMissionType::Riot: return TEXT("Riot response");
    case ESimCopterMissionType::Rescue: return TEXT("Rooftop rescue");
    case ESimCopterMissionType::Transport: return TEXT("Priority transport");
    case ESimCopterMissionType::Medevac: return TEXT("Medical evacuation");
    case ESimCopterMissionType::Fire: return TEXT("Fire suppression");
    case ESimCopterMissionType::Criminal: return TEXT("Criminal pursuit");
    case ESimCopterMissionType::Speeder: return TEXT("Speeder intercept");
    case ESimCopterMissionType::Traffic: return TEXT("Traffic assistance");
    default: return TEXT("Dispatch");
    }
}
}

AMissionMarker::AMissionMarker()
{
    PrimaryActorTick.bCanEverTick = true;

    Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
    Trigger->InitSphereRadius(280.0f);
    Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    RootComponent = Trigger;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(
        TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    Beacon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Beacon"));
    Beacon->SetupAttachment(RootComponent);
    Beacon->SetStaticMesh(CylinderMesh.Object);
    Beacon->SetRelativeScale3D(FVector(0.45f, 0.45f, 4.0f));
    Beacon->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
    Light->SetupAttachment(RootComponent);
    Light->SetLightColor(FLinearColor(1.0f, 0.08f, 0.02f));
    Light->SetIntensity(12000.0f);
    Light->SetAttenuationRadius(1800.0f);
}

void AMissionMarker::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    AddActorWorldRotation(FRotator(0.0f, 45.0f * DeltaSeconds, 0.0f));
    const float Pulse = 0.8f + FMath::Sin(GetWorld()->GetTimeSeconds() * 3.0f) * 0.2f;
    Beacon->SetRelativeScale3D(FVector(0.45f * Pulse, 0.45f * Pulse, 4.0f));
}

AMissionDirector::AMissionDirector()
{
    PrimaryActorTick.bCanEverTick = true;

    static ConstructorHelpers::FObjectFinder<USoundBase> MissionComplete(
        TEXT("/Game/Original/Audio/S_MissionComplete.S_MissionComplete"));
    CompletionSound = MissionComplete.Object;
}

void AMissionDirector::BeginPlay()
{
    Super::BeginPlay();
    PlayerHelicopter = Cast<AHelicopterPawn>(UGameplayStatics::GetPlayerPawn(this, 0));
    StartNextMission();
}

void AMissionDirector::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!PlayerHelicopter)
    {
        PlayerHelicopter = Cast<AHelicopterPawn>(UGameplayStatics::GetPlayerPawn(this, 0));
    }

    if (!ActiveMarker)
    {
        NextMissionDelay -= DeltaSeconds;
        if (NextMissionDelay <= 0.0f)
        {
            StartNextMission();
        }
        return;
    }

    if (PlayerHelicopter)
    {
        const float Distance = FVector::Dist(PlayerHelicopter->GetActorLocation(), ActiveMarker->GetActorLocation());
        const bool bControlledApproach = PlayerHelicopter->GetFlightVelocity().Size() < 550.0f;
        if (Distance < 330.0f && bControlledApproach)
        {
            CompleteMission();
        }
    }
}

FString AMissionDirector::GetMissionSummary() const
{
    if (!ActiveMarker)
    {
        return FString::Printf(TEXT("Mission complete! Dispatching next call... (%d completed)"), CompletedMissions);
    }
    return FString::Printf(TEXT("%s - approach the red beacon slowly"), MissionTypeName(ActiveType));
}

float AMissionDirector::GetMissionDistanceMeters() const
{
    if (!ActiveMarker || !PlayerHelicopter)
    {
        return 0.0f;
    }
    return FVector::Dist(PlayerHelicopter->GetActorLocation(), ActiveMarker->GetActorLocation()) / 100.0f;
}

void AMissionDirector::RestoreProgress(int32 InCompletedMissions)
{
    if (ActiveMarker)
    {
        ActiveMarker->Destroy();
        ActiveMarker = nullptr;
    }
    CompletedMissions = FMath::Max(0, InCompletedMissions);
    NextMissionDelay = 0.0f;
    StartNextMission();
}

void AMissionDirector::StartNextMission()
{
    if (ActiveMarker)
    {
        return;
    }

    ActiveType = static_cast<ESimCopterMissionType>(CompletedMissions % 9);
    const float Angle = FMath::DegreesToRadians(37.0f * CompletedMissions + 25.0f);
    const float Radius = 4200.0f + static_cast<float>((CompletedMissions % 3) * 900);
    const FVector Location(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 260.0f);
    ActiveMarker = GetWorld()->SpawnActor<AMissionMarker>(Location, FRotator::ZeroRotator);
}

void AMissionDirector::CompleteMission()
{
    if (ActiveMarker)
    {
        ActiveMarker->Destroy();
        ActiveMarker = nullptr;
    }
    ++CompletedMissions;
    if (CompletionSound)
    {
        UGameplayStatics::PlaySound2D(this, CompletionSound, 0.65f);
    }
    NextMissionDelay = 3.0f;
}
