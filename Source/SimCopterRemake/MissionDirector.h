#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MissionDirector.generated.h"

class AHelicopterPawn;
class UPointLightComponent;
class USphereComponent;
class UStaticMeshComponent;
class USoundBase;

UENUM(BlueprintType)
enum class ESimCopterMissionType : uint8
{
    General,
    Riot,
    Rescue,
    Transport,
    Medevac,
    Fire,
    Criminal,
    Speeder,
    Traffic
};

UCLASS()
class SIMCOPTERREMAKE_API AMissionMarker : public AActor
{
    GENERATED_BODY()

public:
    AMissionMarker();
    virtual void Tick(float DeltaSeconds) override;

private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USphereComponent> Trigger;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> Beacon;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UPointLightComponent> Light;
};

UCLASS()
class SIMCOPTERREMAKE_API AMissionDirector : public AActor
{
    GENERATED_BODY()

public:
    AMissionDirector();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintPure, Category = "Mission")
    FString GetMissionSummary() const;

    UFUNCTION(BlueprintPure, Category = "Mission")
    float GetMissionDistanceMeters() const;

    UFUNCTION(BlueprintPure, Category = "Mission")
    int32 GetCompletedMissions() const { return CompletedMissions; }

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void RestoreProgress(int32 InCompletedMissions);

private:
    void StartNextMission();
    void CompleteMission();

    UPROPERTY()
    TObjectPtr<AMissionMarker> ActiveMarker;

    UPROPERTY()
    TObjectPtr<AHelicopterPawn> PlayerHelicopter;

    UPROPERTY()
    TObjectPtr<USoundBase> CompletionSound;

    ESimCopterMissionType ActiveType = ESimCopterMissionType::Rescue;
    int32 CompletedMissions = 0;
    float NextMissionDelay = 0.0f;
};
