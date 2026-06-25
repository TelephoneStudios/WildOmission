// Copyright Telephone Studios. All Rights Reserved.


#include "WildOmissionGameUserSettings.h"

static const int32 CURRENT_SETTINGS_VERSION = 0;

static const float DEFAULT_MASTERVOLUME = 1.0f;
static const float DEFAULT_VOLUME = 1.0f;

static const float DEFAULT_FIELDOFVIEW = 90.0f;
static const uint8 DEFAULT_RENDERDISTANCE = 16;
static const bool DEFAULT_SHOWBRANDING = true;
static const bool DEFAULT_SHOWCROSSHAIR = true;
static const bool DEFAULT_HIDECHATUNLESSOPEN = false;
static const bool DEFAULT_HIDEHUD = false;
static const bool DEFAULT_CAMERASHAKEENABLED = true;

static const float DEFAULT_GAMMA = 100.0f;
static const bool DEFAULT_AUTOEXPOSUREENABLED = true;
static const bool DEFAULT_MOTIONBLURENABLED = false;
static const bool DEFAULT_BLOOMENABLED = true;
static const bool DEFAULT_AMBIENTOCCLUSIONENABLED = true;
static const bool DEFAULT_FILMGRAINENABLED = false;

static const FKey DEFAULT_MOVEFORWARD = EKeys::W;
static const FKey DEFAULT_MOVEBACKWARD = EKeys::S;
static const FKey DEFAULT_MOVELEFT = EKeys::A;
static const FKey DEFAULT_MOVERIGHT = EKeys::D;
static const FKey DEFAULT_SPRINT = EKeys::LeftShift;
static const FKey DEFAULT_CROUCH = EKeys::LeftControl;
static const FKey DEFAULT_JUMP = EKeys::SpaceBar;
static const FKey DEFAULT_FLY = EKeys::F;

static const FKey DEFAULT_PRIMARY = EKeys::LeftMouseButton;
static const FKey DEFAULT_SECONDARY = EKeys::RightMouseButton;
static const FKey DEFAULT_INTERACT = EKeys::E;
static const FKey DEFAULT_GRAB = EKeys::G;
static const FKey DEFAULT_RELOAD = EKeys::R;

static const FKey DEFAULT_INVENTORY = EKeys::Tab;
static const FKey DEFAULT_CRAFTING = EKeys::Q;
static const FKey DEFAULT_CHAT = EKeys::T;

static const bool DEFAULT_INVERTEDMOUSEY = false;
static const bool DEFAULT_ALPHATOOLBARSCROLL = false;
static const float DEFAULT_MOUSESENSITIVITY = 1.0f;

UWildOmissionGameUserSettings::UWildOmissionGameUserSettings(const FObjectInitializer& ObjectInitializer) : UGameUserSettings(ObjectInitializer)
{
    SettingsVersion = -1;

    SetWOSettingsDefault();
    
    SetHasRunAutoConfig(false);
}

void UWildOmissionGameUserSettings::LoadSettings(bool bForceReload)
{
    Super::LoadSettings(bForceReload);

    if (SettingsVersion == CURRENT_SETTINGS_VERSION)
    {
        return;
    }

    SetToDefaults();
}

void UWildOmissionGameUserSettings::SetToDefaults()
{
    Super::SetToDefaults();

    SettingsVersion = CURRENT_SETTINGS_VERSION;

    SetWOSettingsDefault();
}

void UWildOmissionGameUserSettings::SetMasterVolume(float Volume)
{
    MasterVolume = Volume;
}

float UWildOmissionGameUserSettings::GetMasterVolume() const
{
    return MasterVolume;
}

void UWildOmissionGameUserSettings::SetMusicVolume(float Volume)
{
    MusicVolume = Volume;
}

float UWildOmissionGameUserSettings::GetMusicVolume() const
{
    return MusicVolume;
}

void UWildOmissionGameUserSettings::SetDeployablesVolume(float Volume)
{
    DeployablesVolume = Volume;
}

float UWildOmissionGameUserSettings::GetDeployablesVolume() const
{
    return DeployablesVolume;
}

void UWildOmissionGameUserSettings::SetEnvironmentVolume(float Volume)
{
    EnvironmentVolume = Volume;
}

float UWildOmissionGameUserSettings::GetEnvironmentVolume() const
{
    return EnvironmentVolume;
}

void UWildOmissionGameUserSettings::SetFriendlyCreaturesVolume(float Volume)
{
    FriendlyCreaturesVolume = Volume;
}

float UWildOmissionGameUserSettings::GetFriendlyCreaturesVolume() const
{
    return FriendlyCreaturesVolume;
}

void UWildOmissionGameUserSettings::SetHostileCreaturesVolume(float Volume)
{
    HostileCreaturesVolume = Volume;
}

float UWildOmissionGameUserSettings::GetHostileCreaturesVolume() const
{
    return HostileCreaturesVolume;
}

void UWildOmissionGameUserSettings::SetPlayersVolume(float Volume)
{
    PlayersVolume = Volume;
}

float UWildOmissionGameUserSettings::GetPlayersVolume() const
{
    return PlayersVolume;
}

void UWildOmissionGameUserSettings::SetWeatherVolume(float Volume)
{
    WeatherVolume = Volume;
}

float UWildOmissionGameUserSettings::GetWeatherVolume() const
{
    return WeatherVolume;
}


void UWildOmissionGameUserSettings::SetShowBranding(bool Show)
{
    ShowBranding = Show;
}

bool UWildOmissionGameUserSettings::GetShowBranding() const
{
    return ShowBranding;
}

void UWildOmissionGameUserSettings::SetShowCrosshair(bool Show)
{
    ShowCrosshair = Show;
}

bool UWildOmissionGameUserSettings::GetShowCrosshair() const
{
    return ShowCrosshair;
}

void UWildOmissionGameUserSettings::SetHideChatUnlessOpen(bool Hide)
{
    HideChatUnlessOpen = Hide;
}

bool UWildOmissionGameUserSettings::GetHideChatUnlessOpen() const
{
    return HideChatUnlessOpen;
}

void UWildOmissionGameUserSettings::SetHideHUD(bool Hide)
{
    HideHUD = Hide;
}

bool UWildOmissionGameUserSettings::GetHideHUD() const
{
    return HideHUD;
}

void UWildOmissionGameUserSettings::SetCameraShakeEnabled(bool Enabled)
{
    CameraShakeEnabled = Enabled;
}

bool UWildOmissionGameUserSettings::GetCameraShakeEnabled() const
{
    return CameraShakeEnabled;
}

void UWildOmissionGameUserSettings::SetFieldOfView(float NewFieldOfView)
{
    FieldOfView = NewFieldOfView;
}

float UWildOmissionGameUserSettings::GetFieldOfView() const
{
    return FieldOfView;
}

void UWildOmissionGameUserSettings::SetRenderDistance(const uint8& NewRenderDistance)
{
    RenderDistance = NewRenderDistance;
}

uint8 UWildOmissionGameUserSettings::GetRenderDistance() const
{
    return RenderDistance;
}

void UWildOmissionGameUserSettings::SetGamma(float NewGamma)
{
    Gamma = NewGamma;
}

float UWildOmissionGameUserSettings::GetGamma() const
{
    return Gamma;
}

void UWildOmissionGameUserSettings::SetAutoExposureEnabled(bool UseAutoExposure)
{
    AutoExposureEnabled = UseAutoExposure;
}

bool UWildOmissionGameUserSettings::GetAutoExposureEnabled() const
{
    return AutoExposureEnabled;
}

void UWildOmissionGameUserSettings::SetMotionBlurEnabled(bool UseMotionBlur)
{
    MotionBlurEnabled = UseMotionBlur;
}

bool UWildOmissionGameUserSettings::GetMotionBlurEnabled() const
{
    return MotionBlurEnabled;
}

void UWildOmissionGameUserSettings::SetBloomEnabled(bool UseBloom)
{
    BloomEnabled = UseBloom;
}

bool UWildOmissionGameUserSettings::GetBloomEnabled() const
{
    return BloomEnabled;
}

void UWildOmissionGameUserSettings::SetAmbientOcclusionEnabled(bool UseAmbientOcclusion)
{
    AmbientOcclusionEnabled = UseAmbientOcclusion;
}

bool UWildOmissionGameUserSettings::GetAmbientOcclusionEnabled() const
{
    return AmbientOcclusionEnabled;
}

void UWildOmissionGameUserSettings::SetFilmGrainEnabled(bool UseFilmGrain)
{
    FilmGrainEnabled = UseFilmGrain;
}

bool UWildOmissionGameUserSettings::GetFilmGrainEnabled() const
{
    return FilmGrainEnabled;
}

void UWildOmissionGameUserSettings::SetMoveForwardKey(const FKey& NewKey)
{
    MoveForwardKey = NewKey;
}

FKey UWildOmissionGameUserSettings::GetMoveForwardKey() const
{
    return MoveForwardKey;
}

void UWildOmissionGameUserSettings::SetMoveBackwardKey(const FKey& NewKey)
{
    MoveBackwardKey = NewKey;
}

FKey UWildOmissionGameUserSettings::GetMoveBackwardKey() const
{
    return MoveBackwardKey;
}

void UWildOmissionGameUserSettings::SetMoveLeftKey(const FKey& NewKey)
{
    MoveLeftKey = NewKey;
}

FKey UWildOmissionGameUserSettings::GetMoveLeftKey() const
{
    return MoveLeftKey;
}

void UWildOmissionGameUserSettings::SetMoveRightKey(const FKey& NewKey)
{
    MoveRightKey = NewKey;
}

FKey UWildOmissionGameUserSettings::GetMoveRightKey() const
{
    return MoveRightKey;
}

void UWildOmissionGameUserSettings::SetSprintKey(const FKey& NewKey)
{
    SprintKey = NewKey;
}

FKey UWildOmissionGameUserSettings::GetSprintKey() const
{
    return SprintKey;
}

void UWildOmissionGameUserSettings::SetCrouchKey(const FKey& NewKey)
{
    CrouchKey = NewKey;
}

FKey UWildOmissionGameUserSettings::GetCrouchKey() const
{
    return CrouchKey;
}

void UWildOmissionGameUserSettings::SetJumpKey(const FKey& NewKey)
{
    JumpKey = NewKey;
}

FKey UWildOmissionGameUserSettings::GetJumpKey() const
{
    return JumpKey;
}

void UWildOmissionGameUserSettings::SetFlyKey(const FKey& NewKey)
{
    FlyKey = NewKey;
}

FKey UWildOmissionGameUserSettings::GetFlyKey() const
{
    return FlyKey;
}

void UWildOmissionGameUserSettings::SetPrimaryKey(const FKey& NewKey)
{
    PrimaryKey = NewKey;
}

FKey UWildOmissionGameUserSettings::GetPrimaryKey() const
{
    return PrimaryKey;
}

void UWildOmissionGameUserSettings::SetSecondaryKey(const FKey& NewKey)
{
    SecondaryKey = NewKey;
}

FKey UWildOmissionGameUserSettings::GetSecondaryKey() const
{
    return SecondaryKey;
}

void UWildOmissionGameUserSettings::SetInteractKey(const FKey& NewKey)
{
    InteractKey = NewKey;
}

FKey UWildOmissionGameUserSettings::GetInteractKey() const
{
    return InteractKey;
}

void UWildOmissionGameUserSettings::SetGrabKey(const FKey& NewKey)
{
    GrabKey = NewKey;
}

FKey UWildOmissionGameUserSettings::GetGrabKey() const
{
    return GrabKey;
}

void UWildOmissionGameUserSettings::SetReloadKey(const FKey& NewKey)
{
    ReloadKey = NewKey;
}

FKey UWildOmissionGameUserSettings::GetReloadKey() const
{
    return ReloadKey;
}

void UWildOmissionGameUserSettings::SetInventoryKey(const FKey& NewKey)
{
    InventoryKey = NewKey;
}

FKey UWildOmissionGameUserSettings::GetInventoryKey() const
{
    return InventoryKey;
}

void UWildOmissionGameUserSettings::SetCraftingKey(const FKey& NewKey)
{
    CraftingKey = NewKey;
}

FKey UWildOmissionGameUserSettings::GetCraftingKey() const
{
    return CraftingKey;
}

void UWildOmissionGameUserSettings::SetChatKey(const FKey& NewKey)
{
    ChatKey = NewKey;
}

FKey UWildOmissionGameUserSettings::GetChatKey() const
{
    return ChatKey;
}

void UWildOmissionGameUserSettings::SetInvertedMouseY(bool Inverted)
{
    InvertedMouseY = Inverted;
}

bool UWildOmissionGameUserSettings::GetInvertedMouseY() const
{
    return InvertedMouseY;
}

void UWildOmissionGameUserSettings::SetAlphaToolbarScroll(bool UseAlpha)
{
    AlphaToolbarScroll = UseAlpha;
}

bool UWildOmissionGameUserSettings::GetAlphaToolbarScroll() const
{
    return AlphaToolbarScroll;
}

void UWildOmissionGameUserSettings::SetMouseSensitivity(float Sensitivity)
{
    MouseSensitivity = Sensitivity;
}

float UWildOmissionGameUserSettings::GetMouseSensitivity() const
{
    return MouseSensitivity;
}

void UWildOmissionGameUserSettings::SetHasRunAutoConfig(bool HasRun)
{
    HasRunAutoConfig = HasRun;
}

bool UWildOmissionGameUserSettings::GetHasRunAutoConfig() const
{
    return HasRunAutoConfig;
}

int32 UWildOmissionGameUserSettings::GetResolutionScaleAsInt32() const
{
    float CurrentScaleNormalized = 0.0f;
    float CurrentScale = 0.0f;
    float MinScale = 0.0f;
    float MaxScale = 0.0f;
    GetResolutionScaleInformationEx(CurrentScaleNormalized, CurrentScale, MinScale, MaxScale);
    return FMath::RoundToInt32(CurrentScale);
}

UWildOmissionGameUserSettings* UWildOmissionGameUserSettings::GetWildOmissionGameUserSettings()
{
    return Cast<UWildOmissionGameUserSettings>(UGameUserSettings::GetGameUserSettings());
}

void UWildOmissionGameUserSettings::SetWOSettingsDefault()
{
    // Audio
    SetMasterVolume(DEFAULT_MASTERVOLUME);
    SetMusicVolume(DEFAULT_VOLUME);
    SetDeployablesVolume(DEFAULT_VOLUME);
    SetEnvironmentVolume(DEFAULT_VOLUME);
    SetFriendlyCreaturesVolume(DEFAULT_VOLUME);
    SetHostileCreaturesVolume(DEFAULT_VOLUME);
    SetPlayersVolume(DEFAULT_VOLUME);
    SetWeatherVolume(DEFAULT_VOLUME);

    // Gameplay
    SetFieldOfView(DEFAULT_FIELDOFVIEW);
    SetRenderDistance(DEFAULT_RENDERDISTANCE);
    SetShowBranding(DEFAULT_SHOWBRANDING);
    SetShowCrosshair(DEFAULT_SHOWCROSSHAIR);
    SetHideChatUnlessOpen(DEFAULT_HIDECHATUNLESSOPEN);
    SetHideHUD(DEFAULT_HIDEHUD);
    SetCameraShakeEnabled(DEFAULT_CAMERASHAKEENABLED);

    // Post Processing
    SetGamma(DEFAULT_GAMMA);
    SetAutoExposureEnabled(DEFAULT_AUTOEXPOSUREENABLED);
    SetMotionBlurEnabled(DEFAULT_MOTIONBLURENABLED);
    SetBloomEnabled(DEFAULT_BLOOMENABLED);
    SetAmbientOcclusionEnabled(DEFAULT_AMBIENTOCCLUSIONENABLED);
    SetFilmGrainEnabled(DEFAULT_FILMGRAINENABLED);

    // Controls
    SetMoveForwardKey(DEFAULT_MOVEFORWARD);
    SetMoveBackwardKey(DEFAULT_MOVEBACKWARD);
    SetMoveLeftKey(DEFAULT_MOVELEFT);
    SetMoveRightKey(DEFAULT_MOVERIGHT);
    SetSprintKey(DEFAULT_SPRINT);
    SetCrouchKey(DEFAULT_CROUCH);
    SetJumpKey(DEFAULT_JUMP);
    SetFlyKey(DEFAULT_FLY);

    SetPrimaryKey(DEFAULT_PRIMARY);
    SetSecondaryKey(DEFAULT_SECONDARY);
    SetInteractKey(DEFAULT_INTERACT);
    SetGrabKey(DEFAULT_GRAB);
    SetReloadKey(DEFAULT_RELOAD);

    SetInventoryKey(DEFAULT_INVENTORY);
    SetCraftingKey(DEFAULT_CRAFTING);
    SetChatKey(DEFAULT_CHAT);

    SetInvertedMouseY(DEFAULT_INVERTEDMOUSEY);
    SetAlphaToolbarScroll(DEFAULT_ALPHATOOLBARSCROLL);
    SetMouseSensitivity(DEFAULT_MOUSESENSITIVITY);

}
