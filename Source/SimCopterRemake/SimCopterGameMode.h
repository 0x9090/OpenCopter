#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SimCopterGameMode.generated.h"

UCLASS()
class SIMCOPTERREMAKE_API ASimCopterGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ASimCopterGameMode();

protected:
    virtual void BeginPlay() override;
};
