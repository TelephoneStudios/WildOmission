// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/MenuInterface.h"
#include "Interfaces/GameSettingsInterface.h"
#include "Interfaces/GameSaveLoadController.h"
#include "WildOmissionGameInstance.generated.h"

class UMainMenuWidget;
class UGameplayMenuWidget;
class ULoadingMenuWidget;
class FOnlineSessionSearch;

USTRUCT()
struct FGameError
{
	GENERATED_BODY()

	bool Acknowlaged;
	FString Title;
	FString Message;

	FGameError();
	void Set(const FString& ErrorTitle, const FString& ErrorMessage);
	void Clear();

};

UCLASS()
class WILDOMISSIONCORE_API UWildOmissionGameInstance : public UGameInstance, public IMenuInterface, public IGameSettingsInterface, public IGameSaveLoadController
{
	GENERATED_BODY()

public:
	UWildOmissionGameInstance(const FObjectInitializer& ObjectInitializer);
	static UWildOmissionGameInstance* GetWildOmissionGameInstance(UWorld* WorldContextObject);

	virtual void Init() override;
	virtual void Shutdown() override;

	UFUNCTION(BlueprintCallable)
	void ShowMainMenuWidget();

	UFUNCTION(BlueprintCallable, Exec)
	void ShowGameplayMenuWidget();

	// Begin IGameSaveLoadController Implementation
	virtual void StartLoading() override;
	virtual void StopLoading() override;
	virtual void SetLoadingTitle(const FString& InLoadingTitle) override;
	virtual void SetLoadingSubtitle(const FString& InLoadingSubtitle) override;
	virtual void CreateWorld(const FString& WorldName, const FString& SeedOverride = TEXT("")) override;
	virtual bool DoesWorldAlreadyExist(const FString& WorldName) const override;
	virtual void RenameWorld(const FString& OldWorldName, const FString& NewWorldName) override;
	virtual void DeleteWorld(const FString& WorldName) override;
	// End IGameSaveLoadController Implementation

	// Begin IMenuInterface Implementation
	virtual void StartSingleplayer(const FString& WorldName, const uint8& GameMode) override;
	virtual void HostServer(const FString& ServerName, const FString& WorldName, bool FriendsOnly = false, const int32& MaxPlayerCount = 8, const uint8& GameMode = 0) override;
	virtual void JoinServer(const uint32& Index) override;
	virtual void RefreshServerList(bool IsDedicated = false) override;
	virtual void QuitToMenu() override;
	// End IMenuInterface Implementation

	void StartSession();

	UFUNCTION(Exec)
	void OpenLevel(const FString& Address);

	// Begin IGameSettingsInterface Implementation
	virtual void ApplyAudioSettings() override;
	// End IGameSettingsInterface Implementation

	IOnlineFriendsPtr GetFriendsInterface() const;

	static FName GetSessionName();
	static FName GetServerNameSettingsKey();
	static FName GetFriendsOnlySettingsKey();
	static FName GetLevelFileSettingsKey();
	static FName GetGameVersionSettingsKey();
	static FName GetSearchPresence();

	UFUNCTION(BlueprintCallable)
	FString GetVersion() const;

	UMainMenuWidget* GetMainMenuWidget() const;
	UGameplayMenuWidget* GetGameplayMenuWidget() const;

private:
	UPROPERTY()
	TSubclassOf<UMainMenuWidget> MainMenuWidgetBlueprintClass;
	UPROPERTY()
	UMainMenuWidget* MainMenuWidget;

	UPROPERTY()
	TSubclassOf<UGameplayMenuWidget> GameplayMenuWidgetBlueprintClass;
	UPROPERTY()
	UGameplayMenuWidget* GameplayMenuWidget;
	UFUNCTION()
	void ClearGameplayMenuWidget();

	// Will try to auto-configure graphics settings if first launch of game
	UFUNCTION(Exec)
	void RunAutoConfigQualitySettings(bool Override = false);

	FGameError LastError;

	UPROPERTY()
	TSubclassOf<ULoadingMenuWidget> LoadingMenuWidgetBlueprintClass;
	UPROPERTY()
	ULoadingMenuWidget* LoadingMenuWidget;
	
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	IOnlineFriendsPtr FriendsInterface;

	UPROPERTY()
	class UServerAdministrators* ServerAdministrators;

	UPROPERTY()
	class UAchievementsManager* AchievementsManager;

	UPROPERTY()
	class UWorkshopManager* WorkshopManager;

	void OnCreateDedicatedSessionComplete(FName SessionName, bool Success);
	void OnCreateSessionComplete(FName SessionName, bool Success);
	void OnDestroySessionComplete(FName SessionName, bool Success);
	void OnFindSessionsComplete(bool Success);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

	FString DesiredServerName;
	FString WorldToLoad;
	bool FriendsOnlySession;
	int32 DesiredMaxPlayerCount;
	uint8 GameModeIndex;

	UFUNCTION()
	void CreateSession(FName SessionName = FName(""), bool Success = true);

	UFUNCTION()
	void LoadedNewMap(UWorld* InWorld);

	void EndExistingSession();

	bool OnMainMenu;
	bool Loading;
	FString LoadingTitle;
	FString LoadingSubtitle;

};
