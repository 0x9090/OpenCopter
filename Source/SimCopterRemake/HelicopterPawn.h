#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HelicopterPawn.generated.h"

class UCameraComponent;
class UAudioComponent;
class UCapsuleComponent;
class USpringArmComponent;
class UStaticMeshComponent;

USTRUCT(BlueprintType)
struct FHelicopterFlightTuning
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
    float MaxForwardSpeed = 2400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
    float MaxStrafeSpeed = 1300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
    float MaxClimbSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
    float YawRateDegrees = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
    float VelocityResponse = 2.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
    float MaxVisualPitch = 18.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
    float MaxVisualRoll = 28.0f;
};

UCLASS()
class SIMCOPTERREMAKE_API AHelicopterPawn : public APawn
{
    GENERATED_BODY()

public:
    AHelicopterPawn();

    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintPure, Category = "Flight")
    float GetAirspeedKph() const;

    UFUNCTION(BlueprintPure, Category = "Flight")
    float GetAltitudeMeters() const;

    UFUNCTION(BlueprintPure, Category = "Flight")
    FVector GetFlightVelocity() const { return FlightVelocity; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCapsuleComponent> Collision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> Body;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> MainRotor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCameraComponent> Camera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UAudioComponent> RotorAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
    FHelicopterFlightTuning Tuning;

private:
    void SetPitchInput(float Value);
    void SetRollInput(float Value);
    void SetCollectiveInput(float Value);
    void SetYawInput(float Value);
    void ToggleMenu();
    void MenuClick();
    void MenuAccept();
    void QuickSave();
    void QuickLoad();

    float PitchInput = 0.0f;
    float RollInput = 0.0f;
    float CollectiveInput = 0.0f;
    float YawInput = 0.0f;
    float AltitudeMeters = 0.0f;
    FVector FlightVelocity = FVector::ZeroVector;
};
