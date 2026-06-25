// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "WildOmissionGameUserSettings.generated.h"

UCLASS()
class GAMESETTINGS_API UWildOmissionGameUserSettings : public UGameUserSettings
{
	GENERATED_UCLASS_BODY()

public:
	virtual void LoadSettings(bool bForceReload = false) override;
	virtual void SetToDefaults() override;

	UFUNCTION()
	void SetMasterVolume(float Volume);

	UFUNCTION()
	float GetMasterVolume() const;

	UFUNCTION()
	void SetMusicVolume(float Volume);

	UFUNCTION()
	float GetMusicVolume() const;

	UFUNCTION()
	void SetDeployablesVolume(float Volume);

	UFUNCTION()
	float GetDeployablesVolume() const;

	UFUNCTION()
	void SetEnvironmentVolume(float Volume);

	UFUNCTION()
	float GetEnvironmentVolume() const;

	UFUNCTION()
	void SetFriendlyCreaturesVolume(float Volume);

	UFUNCTION()
	float GetFriendlyCreaturesVolume() const;

	UFUNCTION()
	void SetHostileCreaturesVolume(float Volume);

	UFUNCTION()
	float GetHostileCreaturesVolume() const;

	UFUNCTION()
	void SetPlayersVolume(float Volume);

	UFUNCTION()
	float GetPlayersVolume() const;

	UFUNCTION()
	void SetWeatherVolume(float Volume);

	UFUNCTION()
	float GetWeatherVolume() const;

	UFUNCTION()
	void SetFieldOfView(float NewFieldOfView);

	UFUNCTION()
	float GetFieldOfView() const;

	UFUNCTION()
	void SetRenderDistance(const uint8& NewRenderDistance);

	UFUNCTION()
	uint8 GetRenderDistance() const;

	UFUNCTION()
	void SetShowBranding(bool Show);

	UFUNCTION()
	bool GetShowBranding() const;

	UFUNCTION()
	void SetShowCrosshair(bool Show);

	UFUNCTION()
	bool GetShowCrosshair() const;

	UFUNCTION()
	void SetHideChatUnlessOpen(bool Hide);

	UFUNCTION()
	bool GetHideChatUnlessOpen() const;

	UFUNCTION()
	void SetHideHUD(bool Hide);

	UFUNCTION()
	bool GetHideHUD() const;

	UFUNCTION()
	void SetCameraShakeEnabled(bool Enabled);

	UFUNCTION()
	bool GetCameraShakeEnabled() const;

	UFUNCTION()
	void SetGamma(float NewGamma);

	UFUNCTION()
	float GetGamma() const;

	UFUNCTION()
	void SetAutoExposureEnabled(bool UseAutoExposure);

	UFUNCTION()
	bool GetAutoExposureEnabled() const;

	UFUNCTION()
	void SetMotionBlurEnabled(bool UseMotionBlur);

	UFUNCTION()
	bool GetMotionBlurEnabled() const;

	UFUNCTION()
	void SetBloomEnabled(bool UseBloom);

	UFUNCTION()
	bool GetBloomEnabled() const;

	UFUNCTION()
	void SetAmbientOcclusionEnabled(bool UseAmbientOcclusion);

	UFUNCTION()
	bool GetAmbientOcclusionEnabled() const;

	UFUNCTION()
	void SetFilmGrainEnabled(bool UseFilmGrain);

	UFUNCTION()
	bool GetFilmGrainEnabled() const;

	UFUNCTION()
	void SetMoveForwardKey(const FKey& NewKey);

	UFUNCTION()
	FKey GetMoveForwardKey() const;

	UFUNCTION()
	void SetMoveBackwardKey(const FKey& NewKey);

	UFUNCTION()
	FKey GetMoveBackwardKey() const;

	UFUNCTION()
	void SetMoveLeftKey(const FKey& NewKey);

	UFUNCTION()
	FKey GetMoveLeftKey() const;
	
	UFUNCTION()
	void SetMoveRightKey(const FKey& NewKey);

	UFUNCTION()
	FKey GetMoveRightKey() const;

	UFUNCTION()
	void SetSprintKey(const FKey& NewKey);

	UFUNCTION()
	FKey GetSprintKey() const;

	UFUNCTION()
	void SetCrouchKey(const FKey& NewKey);

	UFUNCTION()
	FKey GetCrouchKey() const;

	UFUNCTION()
	void SetJumpKey(const FKey& NewKey);

	UFUNCTION()
	FKey GetJumpKey() const;

	UFUNCTION()
	void SetFlyKey(const FKey& NewKey);

	UFUNCTION()
	FKey GetFlyKey() const;

	UFUNCTION()
	void SetPrimaryKey(const FKey& NewKey);

	UFUNCTION()
	FKey GetPrimaryKey() const;

	UFUNCTION()
	void SetSecondaryKey(const FKey& NewKey);

	UFUNCTION()
	FKey GetSecondaryKey() const;

	UFUNCTION()
	void SetInteractKey(const FKey& NewKey);

	UFUNCTION()
	FKey GetInteractKey() const;

	UFUNCTION()
	void SetGrabKey(const FKey& NewKey);

	UFUNCTION()
	FKey GetGrabKey() const;

	UFUNCTION()
	void SetReloadKey(const FKey& NewKey);

	UFUNCTION()
	FKey GetReloadKey() const;

	UFUNCTION()
	void SetInventoryKey(const FKey& NewKey);

	UFUNCTION()
	FKey GetInventoryKey() const;

	UFUNCTION()
	void SetCraftingKey(const FKey& NewKey);

	UFUNCTION()
	FKey GetCraftingKey() const;

	UFUNCTION()
	void SetChatKey(const FKey& NewKey);

	UFUNCTION()
	FKey GetChatKey() const;

	UFUNCTION()
	void SetInvertedMouseY(bool Inverted);

	UFUNCTION()
	bool GetInvertedMouseY() const;

	UFUNCTION()
	void SetAlphaToolbarScroll(bool UseAlpha);

	UFUNCTION()
	bool GetAlphaToolbarScroll() const;

	UFUNCTION()
	void SetMouseSensitivity(float Sensitivity);

	UFUNCTION()
	float GetMouseSensitivity() const;

	UFUNCTION()
	void SetHasRunAutoConfig(bool HasRun);

	UFUNCTION()
	bool GetHasRunAutoConfig() const;

	UFUNCTION()
	int32 GetResolutionScaleAsInt32() const;

	UFUNCTION()
	static UWildOmissionGameUserSettings* GetWildOmissionGameUserSettings();

private:
	UPROPERTY(Config)
	int32 SettingsVersion;

	UPROPERTY(Config)
	float MasterVolume;
	UPROPERTY(Config)
	float MusicVolume;
	UPROPERTY(Config)
	float DeployablesVolume;
	UPROPERTY(Config)
	float EnvironmentVolume;
	UPROPERTY(Config)
	float FriendlyCreaturesVolume;
	UPROPERTY(Config)
	float HostileCreaturesVolume;
	UPROPERTY(Config)
	float PlayersVolume;
	UPROPERTY(Config)
	float WeatherVolume;


	UPROPERTY(Config)
	float FieldOfView;
	UPROPERTY(Config)
	uint8 RenderDistance;
	UPROPERTY(Config)
	bool ShowBranding;
	UPROPERTY(Config)
	bool ShowCrosshair;
	UPROPERTY(Config)
	bool HideChatUnlessOpen;
	UPROPERTY(Config)
	bool HideHUD;
	UPROPERTY(Config)
	bool CameraShakeEnabled;
	
	UPROPERTY(Config)
	float Gamma;
	UPROPERTY(Config)
	bool AutoExposureEnabled;
	UPROPERTY(Config)
	bool MotionBlurEnabled;
	UPROPERTY(Config)
	bool BloomEnabled;
	UPROPERTY(Config)
	bool AmbientOcclusionEnabled;
	UPROPERTY(Config)
	bool FilmGrainEnabled;

	UPROPERTY(Config)
	FKey MoveForwardKey;
	UPROPERTY(Config)
	FKey MoveBackwardKey;
	UPROPERTY(Config)
	FKey MoveLeftKey;
	UPROPERTY(Config)
	FKey MoveRightKey;
	UPROPERTY(Config)
	FKey SprintKey;
	UPROPERTY(Config)
	FKey CrouchKey;
	UPROPERTY(Config)
	FKey JumpKey;
	UPROPERTY(Config)
	FKey FlyKey;
	UPROPERTY(Config)
	FKey PrimaryKey;
	UPROPERTY(Config)
	FKey SecondaryKey;
	UPROPERTY(Config)
	FKey InteractKey;
	UPROPERTY(Config)
	FKey GrabKey;
	UPROPERTY(Config)
	FKey ReloadKey;
	UPROPERTY(Config)
	FKey InventoryKey;
	UPROPERTY(Config)
	FKey CraftingKey;
	UPROPERTY(Config)
	FKey ChatKey;

	UPROPERTY(Config)
	bool InvertedMouseY;
	UPROPERTY(Config)
	bool AlphaToolbarScroll;
	UPROPERTY(Config)
	float MouseSensitivity;

	UPROPERTY(Config)
	bool HasRunAutoConfig;

	UFUNCTION()
	void SetWOSettingsDefault();

};
