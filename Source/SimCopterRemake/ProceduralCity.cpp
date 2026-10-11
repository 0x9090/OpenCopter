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

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(
        TEXT("/Engine/BasicShapes/Plane.Plane"));

    Ground = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ground"));
    Ground->SetupAttachment(SceneRoot);
    Ground->SetStaticMesh(PlaneMesh.Object);
    Ground->SetRelativeScale3D(FVector(140.0f, 140.0f, 1.0f));
    Ground->SetCollisionProfileName(TEXT("BlockAll"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> GroundMaterial(
        TEXT("/Game/Original/Materials/M_OriginalGround.M_OriginalGround"));
    if (GroundMaterial.Succeeded())
    {
        Ground->SetMaterial(0, GroundMaterial.Object);
    }

    Buildings = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Buildings"));
    Buildings->SetupAttachment(SceneRoot);
    Buildings->SetStaticMesh(CubeMesh.Object);
    Buildings->SetCollisionProfileName(TEXT("BlockAll"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> BuildingMaterial(
        TEXT("/Game/Original/Materials/M_OriginalBuilding.M_OriginalBuilding"));
    if (BuildingMaterial.Succeeded())
    {
        Buildings->SetMaterial(0, BuildingMaterial.Object);
    }
}

void AProceduralCity::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    Buildings->ClearInstances();

    FRandomStream Random(1996);
    constexpr int32 HalfGrid = 7;
    constexpr float BlockSpacing = 850.0f;

    for (int32 X = -HalfGrid; X <= HalfGrid; ++X)
    {
        for (int32 Y = -HalfGrid; Y <= HalfGrid; ++Y)
        {
            if (X == 0 || Y == 0 || X % 4 == 0 || Y % 4 == 0)
            {
                continue;
            }

            const float Height = Random.FRandRange(350.0f, 1600.0f);
            const float Width = Random.FRandRange(300.0f, 560.0f);
            const FVector Location(X * BlockSpacing, Y * BlockSpacing, Height * 0.5f);
            const FVector Scale(Width / 100.0f, Width / 100.0f, Height / 100.0f);
            Buildings->AddInstance(FTransform(FRotator::ZeroRotator, Location, Scale));
        }
    }
}
