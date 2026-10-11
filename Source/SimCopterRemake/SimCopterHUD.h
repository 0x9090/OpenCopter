#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SimCopterHUD.generated.h"

class UMaterialInterface;
class USoundBase;

UENUM()
enum class ESimCopterScreen : uint8
{
    MainMenu,
    InGame,
    PauseMenu,
    Controls
};

UCLASS()
class SIMCOPTERREMAKE_API ASimCopterHUD : public AHUD
{
    GENERATED_BODY()

public:
    ASimCopterHUD();

    virtual void BeginPlay() override;
    virtual void DrawHUD() override;
    virtual void NotifyHitBoxClick(FName BoxName) override;

    void TogglePauseMenu();
    void HandleMenuClickAtCursor();
    void ActivatePrimaryMenuAction();
    void SaveCurrentGame();
    void LoadCurrentGame();

private:
    void DrawFlightHUD();
    void DrawMainMenu();
    void DrawPauseMenu();
    void DrawControls();
    void StartNewGame(bool bCareer);
    void ReturnToMainMenu();
    void SetScreen(ESimCopterScreen NewScreen);
    void PlayMenuSound(bool bBack = false);
    void DrawCenteredText(const FString& Text, float CenterX, float Y, const FLinearColor& Color,
        float Scale = 1.0f);
    void AddMenuButton(FName Name, const FString& Label, float X, float Y, float Width, float Height,
        float TextScale = 1.0f);
    void SetStatus(const FString& Message);

    UPROPERTY() TObjectPtr<UMaterialInterface> MainMenuMaterial;
    UPROPERTY() TObjectPtr<UMaterialInterface> PlayMenuMaterial;
    UPROPERTY() TObjectPtr<UMaterialInterface> HangarDayMaterial;
    UPROPERTY() TObjectPtr<UMaterialInterface> HangarNightMaterial;
    UPROPERTY() TObjectPtr<UMaterialInterface> DashboardSideMaterial;
    UPROPERTY() TObjectPtr<UMaterialInterface> DashboardTopMaterial;
    UPROPERTY() TObjectPtr<UMaterialInterface> DashboardRadioMaterial;
    UPROPERTY() TObjectPtr<UMaterialInterface> DashboardLowerMaterial;
    UPROPERTY() TObjectPtr<UMaterialInterface> CompassMaterial;
    UPROPERTY() TObjectPtr<UMaterialInterface> PilotPortraitMaterial;
    UPROPERTY() TObjectPtr<UMaterialInterface> Bell212Material;
    UPROPERTY() TObjectPtr<USoundBase> MenuSelectSound;
    UPROPERTY() TObjectPtr<USoundBase> MenuBackSound;

    ESimCopterScreen Screen = ESimCopterScreen::MainMenu;
    FString StatusMessage;
    float StatusUntilRealTime = 0.0f;
    bool bCareerGame = false;
    static const FString SaveSlot;
};
