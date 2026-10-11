#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SimCopterSaveGame.generated.h"

UCLASS()
class SIMCOPTERREMAKE_API USimCopterSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, Category = "Save")
    FVector HelicopterLocation = FVector(0.0f, 0.0f, 1200.0f);

    UPROPERTY(VisibleAnywhere, Category = "Save")
    FRotator HelicopterRotation = FRotator::ZeroRotator;

    UPROPERTY(VisibleAnywhere, Category = "Save")
    int32 CompletedMissions = 0;

    UPROPERTY(VisibleAnywhere, Category = "Save")
    FString CityName = TEXT("Cape Wells");

    UPROPERTY(VisibleAnywhere, Category = "Save")
    bool bCareerGame = false;
};
