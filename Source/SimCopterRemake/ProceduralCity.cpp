#include "ProceduralCity.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AProceduralCity::AProceduralCity()
{
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(
        TEXT("/Engine/BasicShapes/Plane.Plane"));

    Ground = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ground"));
    Ground->SetupAttachment(SceneRoot);
    Ground->SetStaticMesh(PlaneMesh.Object);
    Ground->SetRelativeScale3D(FVector(1800.0f, 1800.0f, 1.0f));
    Ground->SetCollisionProfileName(TEXT("BlockAll"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> GroundMaterial(
        TEXT("/Game/Original/Materials/M_OriginalGround.M_OriginalGround"));
    if (GroundMaterial.Succeeded())
    {
        Ground->SetMaterial(0, GroundMaterial.Object);
    }

    const TCHAR* OriginalModelPaths[] = {
        TEXT("/Game/Original/Models/AP230.AP230"),
        TEXT("/Game/Original/Models/AR254.AR254"),
        TEXT("/Game/Original/Models/CO182.CO182"),
        TEXT("/Game/Original/Models/ST215.ST215"),
        TEXT("/Game/Original/Models/IN191.IN191"),
        TEXT("/Game/Original/Models/PP204.PP204"),
        TEXT("/Game/Original/Models/PK238.PK238"),
        TEXT("/Game/Original/Models/ML239.ML239"),
    };

    for (int32 Index = 0; Index < UE_ARRAY_COUNT(OriginalModelPaths); ++Index)
    {
        const FName ComponentName(*FString::Printf(TEXT("OriginalBuilding_%d"), Index));
        UInstancedStaticMeshComponent* BuildingSet =
            CreateDefaultSubobject<UInstancedStaticMeshComponent>(ComponentName);
        BuildingSet->SetupAttachment(SceneRoot);
        BuildingSet->SetCollisionProfileName(TEXT("BlockAll"));

        ConstructorHelpers::FObjectFinder<UStaticMesh> OriginalMesh(OriginalModelPaths[Index]);
        if (OriginalMesh.Succeeded())
        {
            BuildingSet->SetStaticMesh(OriginalMesh.Object);
        }
        BuildingSets.Add(BuildingSet);
    }
}

void AProceduralCity::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    for (UInstancedStaticMeshComponent* BuildingSet : BuildingSets)
    {
        BuildingSet->ClearInstances();
    }

    FRandomStream Random(1996);
    constexpr int32 HalfGrid = 7;
    constexpr float BlockSpacing = 10500.0f;

    for (int32 X = -HalfGrid; X <= HalfGrid; ++X)
    {
        for (int32 Y = -HalfGrid; Y <= HalfGrid; ++Y)
        {
            if (X == 0 || Y == 0 || X % 4 == 0 || Y % 4 == 0)
            {
                continue;
            }

            if (BuildingSets.IsEmpty())
            {
                continue;
            }

            const int32 ModelIndex = Random.RandRange(0, BuildingSets.Num() - 1);
            const float Rotation = static_cast<float>(Random.RandRange(0, 3)) * 90.0f;
            const float Scale = Random.FRandRange(0.82f, 1.08f);
            const FVector Location(X * BlockSpacing, Y * BlockSpacing, 0.0f);
            BuildingSets[ModelIndex]->AddInstance(
                FTransform(FRotator(0.0f, Rotation, 0.0f), Location, FVector(Scale)));
        }
    }
}
