#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralCity.generated.h"

class UInstancedStaticMeshComponent;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class SIMCOPTERREMAKE_API AProceduralCity : public AActor
{
    GENERATED_BODY()

public:
    AProceduralCity();

protected:
    virtual void OnConstruction(const FTransform& Transform) override;

private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> Ground;

    UPROPERTY(VisibleAnywhere)
    TArray<TObjectPtr<UInstancedStaticMeshComponent>> BuildingSets;
};
