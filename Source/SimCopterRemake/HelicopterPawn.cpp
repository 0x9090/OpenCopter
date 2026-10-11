#include "HelicopterPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Sound/SoundWave.h"
#include "SimCopterHUD.h"
#include "UObject/ConstructorHelpers.h"

AHelicopterPawn::AHelicopterPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    AutoPossessPlayer = EAutoReceiveInput::Player0;

    Collision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision"));
    Collision->InitCapsuleSize(180.0f, 230.0f);
    Collision->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
    RootComponent = Collision;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> Bell212Mesh(
        TEXT("/Game/Original/Models/BELL212.BELL212"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> BellRotorMesh(
        TEXT("/Game/Original/Models/BELLROTR.BELLROTR"));

    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
    Body->SetupAttachment(RootComponent);
    if (Bell212Mesh.Succeeded())
    {
        Body->SetStaticMesh(Bell212Mesh.Object);
    }
    Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    MainRotor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainRotor"));
    MainRotor->SetupAttachment(Body);
    if (BellRotorMesh.Succeeded())
    {
        MainRotor->SetStaticMesh(BellRotorMesh.Object);
    }
    MainRotor->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    // The recovered Bell 212 is full scale (roughly 17.7 m long), so the
    // original placeholder arm put the camera inside its tail boom.
    CameraBoom->TargetArmLength = 3000.0f;
    CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 350.0f);
    CameraBoom->SetRelativeRotation(FRotator(-14.0f, 0.0f, 0.0f));
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 4.0f;
    CameraBoom->bDoCollisionTest = true;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    Camera->FieldOfView = 80.0f;

    RotorAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("RotorAudio"));
    RotorAudio->SetupAttachment(RootComponent);
    RotorAudio->bAutoActivate = true;
    RotorAudio->SetVolumeMultiplier(0.42f);
    static ConstructorHelpers::FObjectFinder<USoundWave> RotorLoop(
        TEXT("/Game/Original/Audio/S_RotorLoop.S_RotorLoop"));
    if (RotorLoop.Succeeded())
    {
        RotorLoop.Object->bLooping = true;
        RotorAudio->SetSound(RotorLoop.Object);
    }
}

void AHelicopterPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    const FRotator YawOnlyRotation(0.0f, GetActorRotation().Yaw, 0.0f);
    const FVector Forward = YawOnlyRotation.Vector();
    const FVector Right = FRotationMatrix(YawOnlyRotation).GetUnitAxis(EAxis::Y);
    const FVector DesiredVelocity =
        Forward * PitchInput * Tuning.MaxForwardSpeed +
        Right * RollInput * Tuning.MaxStrafeSpeed +
        FVector::UpVector * CollectiveInput * Tuning.MaxClimbSpeed;

    FlightVelocity = FMath::VInterpTo(
        FlightVelocity,
        DesiredVelocity,
        DeltaSeconds,
        Tuning.VelocityResponse);

    AddActorWorldRotation(FRotator(0.0f, YawInput * Tuning.YawRateDegrees * DeltaSeconds, 0.0f));

    FHitResult MoveHit;
    SetActorLocation(GetActorLocation() + FlightVelocity * DeltaSeconds, true, &MoveHit);
    if (MoveHit.IsValidBlockingHit())
    {
        FlightVelocity = FVector::VectorPlaneProject(FlightVelocity, MoveHit.Normal) * 0.35f;
    }

    const FRotator VisualTilt(
        -PitchInput * Tuning.MaxVisualPitch,
        0.0f,
        RollInput * Tuning.MaxVisualRoll);
    Body->SetRelativeRotation(FMath::RInterpTo(
        Body->GetRelativeRotation(),
        VisualTilt,
        DeltaSeconds,
        4.0f));

    MainRotor->AddLocalRotation(FRotator(0.0f, 1440.0f * DeltaSeconds, 0.0f));

    FHitResult GroundHit;
    const FVector TraceStart = GetActorLocation();
    const FVector TraceEnd = TraceStart - FVector(0.0f, 0.0f, 100000.0f);
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(HelicopterAltitude), false, this);
    if (GetWorld()->LineTraceSingleByChannel(
        GroundHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
    {
        AltitudeMeters = FMath::Max(0.0f, (TraceStart.Z - GroundHit.Location.Z) / 100.0f);
    }
}

void AHelicopterPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis(TEXT("Pitch"), this, &AHelicopterPawn::SetPitchInput);
    PlayerInputComponent->BindAxis(TEXT("Roll"), this, &AHelicopterPawn::SetRollInput);
    PlayerInputComponent->BindAxis(TEXT("Collective"), this, &AHelicopterPawn::SetCollectiveInput);
    PlayerInputComponent->BindAxis(TEXT("Yaw"), this, &AHelicopterPawn::SetYawInput);
    FInputActionBinding& MenuBinding = PlayerInputComponent->BindAction(
        TEXT("Menu"), IE_Pressed, this, &AHelicopterPawn::ToggleMenu);
    MenuBinding.bExecuteWhenPaused = true;
    FInputActionBinding& MenuClickBinding = PlayerInputComponent->BindAction(
        TEXT("MenuClick"), IE_Pressed, this, &AHelicopterPawn::MenuClick);
    MenuClickBinding.bExecuteWhenPaused = true;
    FInputActionBinding& MenuAcceptBinding = PlayerInputComponent->BindAction(
        TEXT("MenuAccept"), IE_Pressed, this, &AHelicopterPawn::MenuAccept);
    MenuAcceptBinding.bExecuteWhenPaused = true;
    PlayerInputComponent->BindAction(TEXT("QuickSave"), IE_Pressed, this, &AHelicopterPawn::QuickSave);
    FInputActionBinding& LoadBinding = PlayerInputComponent->BindAction(
        TEXT("QuickLoad"), IE_Pressed, this, &AHelicopterPawn::QuickLoad);
    LoadBinding.bExecuteWhenPaused = true;
}

float AHelicopterPawn::GetAirspeedKph() const
{
    return FlightVelocity.Size2D() * 0.036f;
}

float AHelicopterPawn::GetAltitudeMeters() const
{
    return AltitudeMeters;
}

void AHelicopterPawn::SetPitchInput(float Value)
{
    PitchInput = FMath::Clamp(Value, -1.0f, 1.0f);
}

void AHelicopterPawn::SetRollInput(float Value)
{
    RollInput = FMath::Clamp(Value, -1.0f, 1.0f);
}

void AHelicopterPawn::SetCollectiveInput(float Value)
{
    CollectiveInput = FMath::Clamp(Value, -1.0f, 1.0f);
}

void AHelicopterPawn::SetYawInput(float Value)
{
    YawInput = FMath::Clamp(Value, -1.0f, 1.0f);
}

void AHelicopterPawn::ToggleMenu()
{
    if (APlayerController* Controller = Cast<APlayerController>(GetController()))
    {
        if (ASimCopterHUD* HUD = Cast<ASimCopterHUD>(Controller->GetHUD()))
        {
            HUD->TogglePauseMenu();
        }
    }
}

void AHelicopterPawn::QuickSave()
{
    if (APlayerController* Controller = Cast<APlayerController>(GetController()))
    {
        if (ASimCopterHUD* HUD = Cast<ASimCopterHUD>(Controller->GetHUD()))
        {
            HUD->SaveCurrentGame();
        }
    }
}

void AHelicopterPawn::MenuClick()
{
    if (APlayerController* Controller = Cast<APlayerController>(GetController()))
    {
        if (ASimCopterHUD* HUD = Cast<ASimCopterHUD>(Controller->GetHUD()))
        {
            HUD->HandleMenuClickAtCursor();
        }
    }
}

void AHelicopterPawn::MenuAccept()
{
    if (APlayerController* Controller = Cast<APlayerController>(GetController()))
    {
        if (ASimCopterHUD* HUD = Cast<ASimCopterHUD>(Controller->GetHUD()))
        {
            HUD->ActivatePrimaryMenuAction();
        }
    }
}

void AHelicopterPawn::QuickLoad()
{
    if (APlayerController* Controller = Cast<APlayerController>(GetController()))
    {
        if (ASimCopterHUD* HUD = Cast<ASimCopterHUD>(Controller->GetHUD()))
        {
            HUD->LoadCurrentGame();
        }
    }
}
