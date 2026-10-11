#include "SimCopterHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "HelicopterPawn.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/MaterialInterface.h"
#include "MissionDirector.h"
#include "SimCopterSaveGame.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

const FString ASimCopterHUD::SaveSlot = TEXT("SimCopterSave");

ASimCopterHUD::ASimCopterHUD()
{
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MainMenu(TEXT("/Game/Original/Materials/M_MainMenu.M_MainMenu"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> PlayMenu(TEXT("/Game/Original/Materials/M_PlayMenu.M_PlayMenu"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> HangarDay(TEXT("/Game/Original/Materials/M_HangarDay.M_HangarDay"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> HangarNight(TEXT("/Game/Original/Materials/M_HangarNight.M_HangarNight"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> DashboardSide(TEXT("/Game/Original/Materials/M_DashboardSide.M_DashboardSide"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> DashboardTop(TEXT("/Game/Original/Materials/M_DashboardTop.M_DashboardTop"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> DashboardRadio(TEXT("/Game/Original/Materials/M_DashboardRadio.M_DashboardRadio"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> DashboardLower(TEXT("/Game/Original/Materials/M_DashboardLower.M_DashboardLower"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Compass(TEXT("/Game/Original/Materials/M_Compass.M_Compass"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> PilotPortraits(TEXT("/Game/Original/Materials/M_PilotPortraits.M_PilotPortraits"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Bell212(TEXT("/Game/Original/Materials/M_Bell212.M_Bell212"));
    static ConstructorHelpers::FObjectFinder<USoundBase> SelectSound(TEXT("/Game/Original/Audio/S_MenuSelect.S_MenuSelect"));
    static ConstructorHelpers::FObjectFinder<USoundBase> BackSound(TEXT("/Game/Original/Audio/S_MenuBack.S_MenuBack"));

    MainMenuMaterial = MainMenu.Object;
    PlayMenuMaterial = PlayMenu.Object;
    HangarDayMaterial = HangarDay.Object;
    HangarNightMaterial = HangarNight.Object;
    DashboardSideMaterial = DashboardSide.Object;
    DashboardTopMaterial = DashboardTop.Object;
    DashboardRadioMaterial = DashboardRadio.Object;
    DashboardLowerMaterial = DashboardLower.Object;
    CompassMaterial = Compass.Object;
    PilotPortraitMaterial = PilotPortraits.Object;
    Bell212Material = Bell212.Object;
    MenuSelectSound = SelectSound.Object;
    MenuBackSound = BackSound.Object;
}

void ASimCopterHUD::BeginPlay()
{
    Super::BeginPlay();
    SetScreen(ESimCopterScreen::MainMenu);
}

void ASimCopterHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas)
    {
        return;
    }

    switch (Screen)
    {
    case ESimCopterScreen::MainMenu: DrawMainMenu(); break;
    case ESimCopterScreen::PauseMenu: DrawFlightHUD(); DrawPauseMenu(); break;
    case ESimCopterScreen::Controls: DrawControls(); break;
    default: DrawFlightHUD(); break;
    }

    if (!StatusMessage.IsEmpty() && GetWorld()->GetRealTimeSeconds() < StatusUntilRealTime)
    {
        const float Width = FMath::Min(560.0f, Canvas->ClipX - 40.0f);
        DrawRect(FLinearColor(0.02f, 0.02f, 0.02f, 0.88f), (Canvas->ClipX - Width) * 0.5f,
            Canvas->ClipY - 54.0f, Width, 36.0f);
        DrawCenteredText(StatusMessage, Canvas->ClipX * 0.5f, Canvas->ClipY - 46.0f,
            FLinearColor(0.3f, 0.95f, 1.0f));
    }
}

void ASimCopterHUD::DrawMainMenu()
{
    if (HangarDayMaterial)
    {
        DrawMaterial(HangarDayMaterial, 0.0f, 0.0f, Canvas->ClipX, Canvas->ClipY, 0.0f, 0.0f, 1.0f, 1.0f);
    }
    else
    {
        DrawRect(FLinearColor(0.015f, 0.02f, 0.028f), 0.0f, 0.0f, Canvas->ClipX, Canvas->ClipY);
    }
    DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.20f), 0.0f, 0.0f, Canvas->ClipX, Canvas->ClipY);

    const float Scale = FMath::Clamp(Canvas->ClipY / 540.0f, 0.8f, 1.55f);
    const float PanelWidth = 425.0f * Scale;
    const float PanelHeight = 414.0f * Scale;
    const float PanelX = Canvas->ClipX - PanelWidth - 32.0f * Scale;
    const float PanelY = (Canvas->ClipY - PanelHeight) * 0.5f;
    if (MainMenuMaterial)
    {
        DrawMaterial(MainMenuMaterial, PanelX, PanelY, PanelWidth, PanelHeight, 0.0f, 0.0f, 1.0f, 1.0f);
    }

    const FName Names[] = {TEXT("Career"), TEXT("FreeFlight"), TEXT("Load"), TEXT("Controls"), TEXT("Quit")};
    const TCHAR* Labels[] = {TEXT("START CAREER"), TEXT("FREE FLIGHT"), TEXT("LOAD GAME"), TEXT("CONTROLS"), TEXT("QUIT")};
    for (int32 Index = 0; Index < 5; ++Index)
    {
        AddMenuButton(Names[Index], Labels[Index], PanelX + 91.0f * Scale,
            PanelY + (31.0f + Index * 64.5f) * Scale, 250.0f * Scale, 50.0f * Scale, Scale * 1.05f);
    }

    DrawCenteredText(TEXT("ORIGINAL 1996 ARTWORK EDITION"), PanelX + PanelWidth * 0.5f,
        PanelY + PanelHeight - 24.0f * Scale, FLinearColor(0.78f, 0.84f, 0.86f), Scale * 0.7f);
}

void ASimCopterHUD::DrawPauseMenu()
{
    DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.62f), 0.0f, 0.0f, Canvas->ClipX, Canvas->ClipY);
    const float Scale = FMath::Clamp(Canvas->ClipY / 520.0f, 0.75f, 1.35f);
    const float Width = 343.0f * Scale;
    const float Height = 433.0f * Scale;
    const float X = (Canvas->ClipX - Width) * 0.5f;
    const float Y = (Canvas->ClipY - Height) * 0.5f;
    if (PlayMenuMaterial)
    {
        DrawMaterial(PlayMenuMaterial, X, Y, Width, Height, 0.0f, 0.0f, 1.0f, 1.0f);
    }
    DrawCenteredText(TEXT("FLIGHT PAUSED"), X + Width * 0.5f, Y + 25.0f * Scale,
        FLinearColor(0.35f, 0.95f, 1.0f), Scale * 1.1f);
    const TCHAR* Labels[] = {TEXT("RESUME"), TEXT("SAVE GAME"), TEXT("LOAD GAME"), TEXT("MAIN MENU"), TEXT("QUIT")};
    const FName Names[] = {TEXT("Resume"), TEXT("Save"), TEXT("Load"), TEXT("MainMenu"), TEXT("Quit")};
    for (int32 Index = 0; Index < 5; ++Index)
    {
        AddMenuButton(Names[Index], Labels[Index], X + 87.0f * Scale,
            Y + (66.0f + Index * 48.0f) * Scale, 190.0f * Scale, 39.0f * Scale, Scale);
    }
}

void ASimCopterHUD::DrawControls()
{
    if (HangarNightMaterial)
    {
        DrawMaterial(HangarNightMaterial, 0.0f, 0.0f, Canvas->ClipX, Canvas->ClipY, 0.0f, 0.0f, 1.0f, 1.0f);
    }
    DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.70f), 0.0f, 0.0f, Canvas->ClipX, Canvas->ClipY);
    DrawCenteredText(TEXT("SIMCOPTER FLIGHT CONTROLS"), Canvas->ClipX * 0.5f, 70.0f,
        FLinearColor(0.35f, 0.95f, 1.0f), 1.6f);
    const TCHAR* Lines[] = {
        TEXT("W / S    PITCH FORWARD / REVERSE"), TEXT("A / D    STRAFE AND BANK"),
        TEXT("Q / E    YAW"), TEXT("SPACE / LEFT CTRL    CLIMB / DESCEND"),
        TEXT("ESC    PAUSE MENU"), TEXT("F5 / F9    QUICK SAVE / QUICK LOAD")
    };
    for (int32 Index = 0; Index < 6; ++Index)
    {
        DrawCenteredText(Lines[Index], Canvas->ClipX * 0.5f, 145.0f + Index * 45.0f,
            FLinearColor::White);
    }
    AddMenuButton(TEXT("Back"), TEXT("BACK"), Canvas->ClipX * 0.5f - 115.0f,
        Canvas->ClipY - 105.0f, 230.0f, 52.0f, 1.1f);
}

void ASimCopterHUD::DrawFlightHUD()
{
    const AHelicopterPawn* Helicopter = Cast<AHelicopterPawn>(GetOwningPawn());
    const AMissionDirector* Mission = Cast<AMissionDirector>(UGameplayStatics::GetActorOfClass(this, AMissionDirector::StaticClass()));
    const float Scale = FMath::Clamp(FMath::Min(Canvas->ClipX / 640.0f, Canvas->ClipY / 480.0f), 0.8f, 2.0f);

    if (DashboardTopMaterial)
    {
        DrawMaterial(DashboardTopMaterial, (Canvas->ClipX - 455.0f * Scale) * 0.5f, 0.0f,
            455.0f * Scale, 43.0f * Scale, 0.0f, 0.0f, 1.0f, 1.0f);
    }
    if (DashboardSideMaterial)
    {
        DrawMaterial(DashboardSideMaterial, 0.0f, Canvas->ClipY - 291.0f * Scale,
            84.0f * Scale, 291.0f * Scale, 0.0f, 0.0f, 1.0f, 1.0f);
    }
    if (DashboardRadioMaterial)
    {
        DrawMaterial(DashboardRadioMaterial, Canvas->ClipX - 185.0f * Scale, 12.0f * Scale,
            185.0f * Scale, 148.0f * Scale, 0.0f, 0.0f, 1.0f, 1.0f);
    }
    if (DashboardLowerMaterial)
    {
        DrawMaterial(DashboardLowerMaterial, (Canvas->ClipX - 458.0f * Scale) * 0.5f,
            Canvas->ClipY - 82.0f * Scale, 458.0f * Scale, 82.0f * Scale, 0.0f, 0.0f, 1.0f, 1.0f);
    }
    if (CompassMaterial)
    {
        DrawMaterial(CompassMaterial, (Canvas->ClipX - 160.0f * Scale) * 0.5f,
            5.0f * Scale, 160.0f * Scale, 16.0f * Scale, 0.0f, 0.0f, 1.0f, 1.0f);
    }
    if (PilotPortraitMaterial)
    {
        DrawMaterial(PilotPortraitMaterial, 92.0f * Scale, Canvas->ClipY - 75.0f * Scale,
            54.0f * Scale, 66.0f * Scale, 1.0f / 12.0f, 0.0f, 1.0f / 12.0f, 1.0f / 3.0f);
    }
    if (Helicopter)
    {
        DrawText(FString::Printf(TEXT("SPD %03.0f"), Helicopter->GetAirspeedKph()), FLinearColor(0.28f, 1.0f, 0.42f),
            Canvas->ClipX - 162.0f * Scale, 42.0f * Scale, GEngine->GetSmallFont(), 0.85f * Scale, false);
        DrawText(FString::Printf(TEXT("ALT %03.0f"), Helicopter->GetAltitudeMeters()), FLinearColor(0.28f, 1.0f, 0.42f),
            Canvas->ClipX - 162.0f * Scale, 61.0f * Scale, GEngine->GetSmallFont(), 0.85f * Scale, false);
    }
    if (Mission)
    {
        DrawRect(FLinearColor(0.015f, 0.02f, 0.025f, 0.78f), 100.0f * Scale, 51.0f * Scale,
            FMath::Min(520.0f * Scale, Canvas->ClipX - 310.0f * Scale), 43.0f * Scale);
        DrawText(Mission->GetMissionSummary(), FLinearColor(1.0f, 0.72f, 0.18f), 112.0f * Scale,
            57.0f * Scale, GEngine->GetSmallFont(), 0.78f * Scale, false);
        DrawText(FString::Printf(TEXT("%.0f m"), Mission->GetMissionDistanceMeters()), FLinearColor::White,
            112.0f * Scale, 74.0f * Scale, GEngine->GetSmallFont(), 0.7f * Scale, false);
    }
}

void ASimCopterHUD::NotifyHitBoxClick(FName BoxName)
{
    Super::NotifyHitBoxClick(BoxName);
    if (BoxName == TEXT("Career")) { PlayMenuSound(); StartNewGame(true); }
    else if (BoxName == TEXT("FreeFlight")) { PlayMenuSound(); StartNewGame(false); }
    else if (BoxName == TEXT("Load")) { PlayMenuSound(); LoadCurrentGame(); }
    else if (BoxName == TEXT("Controls")) { PlayMenuSound(); SetScreen(ESimCopterScreen::Controls); }
    else if (BoxName == TEXT("Back")) { PlayMenuSound(true); SetScreen(ESimCopterScreen::MainMenu); }
    else if (BoxName == TEXT("Resume")) { PlayMenuSound(true); SetScreen(ESimCopterScreen::InGame); }
    else if (BoxName == TEXT("Save")) { PlayMenuSound(); SaveCurrentGame(); }
    else if (BoxName == TEXT("MainMenu")) { PlayMenuSound(true); ReturnToMainMenu(); }
    else if (BoxName == TEXT("Quit")) { UKismetSystemLibrary::QuitGame(this, GetOwningPlayerController(), EQuitPreference::Quit, false); }
}

void ASimCopterHUD::TogglePauseMenu()
{
    if (Screen == ESimCopterScreen::InGame) SetScreen(ESimCopterScreen::PauseMenu);
    else if (Screen == ESimCopterScreen::PauseMenu) SetScreen(ESimCopterScreen::InGame);
}

void ASimCopterHUD::HandleMenuClickAtCursor()
{
    if (!Canvas || Screen == ESimCopterScreen::InGame)
    {
        return;
    }

    APlayerController* Controller = GetOwningPlayerController();
    float MouseX = 0.0f;
    float MouseY = 0.0f;
    if (!Controller || !Controller->GetMousePosition(MouseX, MouseY))
    {
        return;
    }

    const auto Hit = [MouseX, MouseY](float X, float Y, float Width, float Height)
    {
        const auto PointHit = [X, Y, Width, Height](float PointX, float PointY)
        {
            return PointX >= X && PointX <= X + Width && PointY >= Y && PointY <= Y + Height;
        };
        // macOS PIE can report cursor coordinates in logical points while Canvas
        // reports backing pixels (or vice versa) depending on Retina scaling.
        return PointHit(MouseX, MouseY) || PointHit(MouseX * 2.0f, MouseY * 2.0f) ||
            PointHit(MouseX * 0.5f, MouseY * 0.5f);
    };

    if (Screen == ESimCopterScreen::MainMenu)
    {
        const float Scale = FMath::Clamp(Canvas->ClipY / 540.0f, 0.8f, 1.55f);
        const float PanelWidth = 425.0f * Scale;
        const float PanelHeight = 414.0f * Scale;
        const float PanelX = Canvas->ClipX - PanelWidth - 32.0f * Scale;
        const float PanelY = (Canvas->ClipY - PanelHeight) * 0.5f;
        const FName Names[] = {TEXT("Career"), TEXT("FreeFlight"), TEXT("Load"), TEXT("Controls"), TEXT("Quit")};
        for (int32 Index = 0; Index < 5; ++Index)
        {
            if (Hit(PanelX + 91.0f * Scale, PanelY + (31.0f + Index * 64.5f) * Scale,
                250.0f * Scale, 50.0f * Scale))
            {
                NotifyHitBoxClick(Names[Index]);
                return;
            }
        }
    }
    else if (Screen == ESimCopterScreen::PauseMenu)
    {
        const float Scale = FMath::Clamp(Canvas->ClipY / 520.0f, 0.75f, 1.35f);
        const float Width = 343.0f * Scale;
        const float Height = 433.0f * Scale;
        const float X = (Canvas->ClipX - Width) * 0.5f;
        const float Y = (Canvas->ClipY - Height) * 0.5f;
        const FName Names[] = {TEXT("Resume"), TEXT("Save"), TEXT("Load"), TEXT("MainMenu"), TEXT("Quit")};
        for (int32 Index = 0; Index < 5; ++Index)
        {
            if (Hit(X + 87.0f * Scale, Y + (66.0f + Index * 48.0f) * Scale,
                190.0f * Scale, 39.0f * Scale))
            {
                NotifyHitBoxClick(Names[Index]);
                return;
            }
        }
    }
    else if (Screen == ESimCopterScreen::Controls &&
        Hit(Canvas->ClipX * 0.5f - 115.0f, Canvas->ClipY - 105.0f, 230.0f, 52.0f))
    {
        NotifyHitBoxClick(TEXT("Back"));
    }
}

void ASimCopterHUD::ActivatePrimaryMenuAction()
{
    if (Screen == ESimCopterScreen::MainMenu) NotifyHitBoxClick(TEXT("Career"));
    else if (Screen == ESimCopterScreen::PauseMenu) NotifyHitBoxClick(TEXT("Resume"));
    else if (Screen == ESimCopterScreen::Controls) NotifyHitBoxClick(TEXT("Back"));
}

void ASimCopterHUD::StartNewGame(bool bCareer)
{
    bCareerGame = bCareer;
    if (AHelicopterPawn* Helicopter = Cast<AHelicopterPawn>(GetOwningPawn()))
    {
        Helicopter->SetActorLocationAndRotation(FVector(0.0f, 0.0f, 1200.0f), FRotator::ZeroRotator,
            false, nullptr, ETeleportType::ResetPhysics);
    }
    if (AMissionDirector* Mission = Cast<AMissionDirector>(UGameplayStatics::GetActorOfClass(this, AMissionDirector::StaticClass())))
    {
        Mission->RestoreProgress(0);
    }
    SetScreen(ESimCopterScreen::InGame);
    SetStatus(bCareer ? TEXT("CAREER STARTED - SEA CLIFF") : TEXT("FREE FLIGHT - CAPE WELLS"));
}

void ASimCopterHUD::SaveCurrentGame()
{
    USimCopterSaveGame* Save = Cast<USimCopterSaveGame>(UGameplayStatics::CreateSaveGameObject(USimCopterSaveGame::StaticClass()));
    if (!Save) { SetStatus(TEXT("SAVE FAILED")); return; }
    if (const AHelicopterPawn* Helicopter = Cast<AHelicopterPawn>(GetOwningPawn()))
    {
        Save->HelicopterLocation = Helicopter->GetActorLocation();
        Save->HelicopterRotation = Helicopter->GetActorRotation();
    }
    if (const AMissionDirector* Mission = Cast<AMissionDirector>(UGameplayStatics::GetActorOfClass(this, AMissionDirector::StaticClass())))
    {
        Save->CompletedMissions = Mission->GetCompletedMissions();
    }
    Save->bCareerGame = bCareerGame;
    Save->CityName = bCareerGame ? TEXT("Sea Cliff") : TEXT("Cape Wells");
    SetStatus(UGameplayStatics::SaveGameToSlot(Save, SaveSlot, 0) ? TEXT("GAME SAVED") : TEXT("SAVE FAILED"));
}

void ASimCopterHUD::LoadCurrentGame()
{
    USimCopterSaveGame* Save = Cast<USimCopterSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlot, 0));
    if (!Save) { SetStatus(TEXT("NO SAVED GAME FOUND")); return; }
    bCareerGame = Save->bCareerGame;
    if (AHelicopterPawn* Helicopter = Cast<AHelicopterPawn>(GetOwningPawn()))
    {
        Helicopter->SetActorLocationAndRotation(Save->HelicopterLocation, Save->HelicopterRotation,
            false, nullptr, ETeleportType::ResetPhysics);
    }
    if (AMissionDirector* Mission = Cast<AMissionDirector>(UGameplayStatics::GetActorOfClass(this, AMissionDirector::StaticClass())))
    {
        Mission->RestoreProgress(Save->CompletedMissions);
    }
    SetScreen(ESimCopterScreen::InGame);
    SetStatus(FString::Printf(TEXT("LOADED %s - %d CALLS COMPLETE"), *Save->CityName.ToUpper(), Save->CompletedMissions));
}

void ASimCopterHUD::ReturnToMainMenu() { SetScreen(ESimCopterScreen::MainMenu); }

void ASimCopterHUD::SetScreen(ESimCopterScreen NewScreen)
{
    Screen = NewScreen;
    const bool bMenu = Screen != ESimCopterScreen::InGame;
    // Keep the title and controls screens ticking so AHUD hit boxes receive mouse
    // events in PIE and packaged builds. Only the in-flight pause screen freezes
    // the simulation; movement/look input is explicitly suppressed for all menus.
    UGameplayStatics::SetGamePaused(this, Screen == ESimCopterScreen::PauseMenu);
    if (APlayerController* Controller = GetOwningPlayerController())
    {
        Controller->ResetIgnoreMoveInput();
        Controller->ResetIgnoreLookInput();
        if (bMenu)
        {
            Controller->SetIgnoreMoveInput(true);
            Controller->SetIgnoreLookInput(true);
        }
        Controller->bEnableClickEvents = bMenu;
        Controller->bEnableMouseOverEvents = bMenu;
        if (bMenu)
        {
            // Menus are rendered by AHUD rather than Slate, so mouse actions must
            // stay on the game input stack. The cursor remains visible and the
            // initial mouse-down is not swallowed by viewport capture.
            FInputModeGameOnly Mode;
            Mode.SetConsumeCaptureMouseDown(false);
            Controller->SetInputMode(Mode);
        }
        else
        {
            Controller->SetInputMode(FInputModeGameOnly());
        }
        Controller->bShowMouseCursor = bMenu;
    }
}

void ASimCopterHUD::PlayMenuSound(bool bBack)
{
    if (USoundBase* Sound = bBack ? MenuBackSound.Get() : MenuSelectSound.Get()) UGameplayStatics::PlaySound2D(this, Sound, 0.7f);
}

void ASimCopterHUD::DrawCenteredText(const FString& Text, float CenterX, float Y, const FLinearColor& Color, float Scale)
{
    float Width = 0.0f, Height = 0.0f;
    GetTextSize(Text, Width, Height, GEngine->GetSmallFont(), Scale);
    DrawText(Text, Color, CenterX - Width * 0.5f, Y, GEngine->GetSmallFont(), Scale, false);
}

void ASimCopterHUD::AddMenuButton(FName Name, const FString& Label, float X, float Y, float Width, float Height, float TextScale)
{
    AddHitBox(FVector2D(X, Y), FVector2D(Width, Height), Name, true, 0);
    DrawCenteredText(Label, X + Width * 0.5f, Y + Height * 0.29f, FLinearColor(0.88f, 0.94f, 0.95f), TextScale);
}

void ASimCopterHUD::SetStatus(const FString& Message)
{
    StatusMessage = Message;
    StatusUntilRealTime = GetWorld()->GetRealTimeSeconds() + 3.0f;
}
