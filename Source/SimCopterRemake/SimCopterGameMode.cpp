#include "SimCopterGameMode.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HelicopterPawn.h"
#include "MissionDirector.h"
#include "ProceduralCity.h"
#include "SimCopterHUD.h"

ASimCopterGameMode::ASimCopterGameMode()
{
    DefaultPawnClass = AHelicopterPawn::StaticClass();
    HUDClass = ASimCopterHUD::StaticClass();
}

void ASimCopterGameMode::BeginPlay()
{
    Super::BeginPlay();

    GetWorld()->SpawnActor<AProceduralCity>(FVector::ZeroVector, FRotator::ZeroRotator);

    ADirectionalLight* Sun = GetWorld()->SpawnActor<ADirectionalLight>(
        FVector(0.0f, 0.0f, 5000.0f), FRotator(-48.0f, -28.0f, 0.0f));
    if (Sun)
    {
        Sun->GetLightComponent()->SetIntensity(10.0f);
        if (UDirectionalLightComponent* SunComponent =
                Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
        {
            SunComponent->SetAtmosphereSunLight(true);
        }
    }

    GetWorld()->SpawnActor<ASkyAtmosphere>(FVector::ZeroVector, FRotator::ZeroRotator);

    ASkyLight* Sky = GetWorld()->SpawnActor<ASkyLight>(FVector::ZeroVector, FRotator::ZeroRotator);
    if (Sky)
    {
        Sky->GetLightComponent()->SetIntensity(1.0f);
        Sky->GetLightComponent()->RecaptureSky();
    }

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    AHelicopterPawn* Helicopter = PlayerController ? Cast<AHelicopterPawn>(PlayerController->GetPawn()) : nullptr;
    if (!Helicopter)
    {
        Helicopter = GetWorld()->SpawnActor<AHelicopterPawn>(
            FVector(0.0f, 0.0f, 1200.0f), FRotator::ZeroRotator);
        if (PlayerController && Helicopter)
        {
            PlayerController->Possess(Helicopter);
        }
    }
    else
    {
        Helicopter->SetActorLocationAndRotation(
            FVector(0.0f, 0.0f, 1200.0f),
            FRotator::ZeroRotator,
            false,
            nullptr,
            ETeleportType::ResetPhysics);
    }

    GetWorld()->SpawnActor<AMissionDirector>(FVector::ZeroVector, FRotator::ZeroRotator);
}
