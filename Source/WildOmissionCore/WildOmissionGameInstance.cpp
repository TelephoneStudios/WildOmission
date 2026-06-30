// Copyright Telephone Studios. All Rights Reserved.


#include "WildOmissionGameInstance.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineFriendsInterface.h" 
#include "SocketSubsystem.h"
#include "Sockets.h"
#include "Serialization/ArrayWriter.h"
#include "UI/MainMenuWidget.h"
#include "UI/GameplayMenuWidget.h"
#include "UI/LoadingMenuWidget.h"
#include "WorldInformation.h"
#include "WorldData.h"
#include "WildOmissionGameUserSettings.h"
#include "AchievementsManager.h"
#include "ServerAdministrators.h"
#include "WorkshopManager.h"
#include "SaveManager.h"
#include "GameFramework/PlayerState.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "Kismet/GameplayStatics.h"

// Static session information
const static FName SESSION_NAME = TEXT("Game");
const static FName SERVER_NAME_SETTINGS_KEY = TEXT("ServerName");
const static FName FRIENDS_ONLY_SETTINGS_KEY = TEXT("FriendsOnlySession");
const static FName LEVEL_FILE_SETTINGS_KEY = TEXT("LevelFile");
const static FName GAME_VERSION_SETTINGS_KEY = TEXT("GameVersion");
const static FName SEARCH_PRESENCE = TEXT("PRESENCESEARCH");
const static FName SEARCH_DEDICATED_ONLY = TEXT("DEDICATEDONLY");

const static FString GameVersion = TEXT("Beta 1.3.0");

static USoundMix* MasterSoundMixModifier = nullptr;
static USoundClass* MasterSoundClass = nullptr;
static USoundClass* MusicSoundClass = nullptr;
static USoundClass* DeployablesSoundClass = nullptr;
static USoundClass* EnvironmentSoundClass = nullptr;
static USoundClass* FriendlyCreaturesSoundClass = nullptr;
static USoundClass* HostileCreaturesSoundClass = nullptr;
static USoundClass* PlayersSoundClass = nullptr;
static USoundClass* WeatherSoundClass = nullptr;

UWildOmissionGameInstance::UWildOmissionGameInstance(const FObjectInitializer& ObjectIntializer)
{
	MainMenuWidgetBlueprintClass = nullptr;
	MainMenuWidget = nullptr;

	GameplayMenuWidgetBlueprintClass = nullptr;
	GameplayMenuWidget = nullptr;

	LoadingMenuWidgetBlueprintClass = nullptr;
	LoadingMenuWidget = nullptr;

	SessionInterface = nullptr;
	SessionSearch = nullptr;

	FriendsInterface = nullptr;

	ServerAdministrators = nullptr;
	AchievementsManager = nullptr;
	WorkshopManager = nullptr;

	DesiredServerName = TEXT("Server");
	WorldToLoad = TEXT("NAN");
	FriendsOnlySession = false;
	DesiredMaxPlayerCount = 8;

	GameModeIndex = 0;

	OnMainMenu = false;
	Loading = false;
	LoadingTitle = TEXT("Loading Title.");
	LoadingSubtitle = TEXT("Loading Subtitle.");

	static ConstructorHelpers::FClassFinder<UMainMenuWidget> MainMenuBlueprint(TEXT("/Game/MenuSystem/UI/WBP_MainMenu"));
	if (MainMenuBlueprint.Succeeded())
	{
		MainMenuWidgetBlueprintClass = MainMenuBlueprint.Class;
	}

	static ConstructorHelpers::FClassFinder<UGameplayMenuWidget> GameplayMenuBlueprint(TEXT("/Game/MenuSystem/UI/WBP_GameplayMenu"));
	if (GameplayMenuBlueprint.Succeeded())
	{
		GameplayMenuWidgetBlueprintClass = GameplayMenuBlueprint.Class;
	}

	static ConstructorHelpers::FClassFinder<ULoadingMenuWidget> LoadingMenuBlueprint(TEXT("/Game/MenuSystem/UI/WBP_LoadingMenu"));
	if (LoadingMenuBlueprint.Succeeded())
	{
		LoadingMenuWidgetBlueprintClass = LoadingMenuBlueprint.Class;
	}

	static ConstructorHelpers::FObjectFinder<USoundMix> MasterSoundMixModifierBlueprint(TEXT("/Game/WildOmissionCore/Audio/MasterSoundClassMix"));
	if (MasterSoundMixModifierBlueprint.Succeeded())
	{
		MasterSoundMixModifier = MasterSoundMixModifierBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundClass> MasterSoundClassBlueprint(TEXT("/Engine/EngineSounds/Master"));
	if (MasterSoundClassBlueprint.Succeeded())
	{
		MasterSoundClass = MasterSoundClassBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundClass> MusicSoundClassBlueprint(TEXT("/Game/WildOmissionCore/Audio/Music"));
	if (MusicSoundClassBlueprint.Succeeded())
	{
		MusicSoundClass = MusicSoundClassBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundClass> DeployablesSoundClassBlueprint(TEXT("/Game/WildOmissionCore/Audio/Deployables"));
	if (DeployablesSoundClassBlueprint.Succeeded())
	{
		DeployablesSoundClass = DeployablesSoundClassBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundClass> EnvironmentSoundClassBlueprint(TEXT("/Game/WildOmissionCore/Audio/Environment"));
	if (EnvironmentSoundClassBlueprint.Succeeded())
	{
		EnvironmentSoundClass = EnvironmentSoundClassBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundClass> FriendlyCreaturesSoundClassBlueprint(TEXT("/Game/WildOmissionCore/Audio/FriendlyCreatures"));
	if (FriendlyCreaturesSoundClassBlueprint.Succeeded())
	{
		FriendlyCreaturesSoundClass = FriendlyCreaturesSoundClassBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundClass> HostileCreaturesSoundClassBlueprint(TEXT("/Game/WildOmissionCore/Audio/HostileCreatures"));
	if (HostileCreaturesSoundClassBlueprint.Succeeded())
	{
		HostileCreaturesSoundClass = HostileCreaturesSoundClassBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundClass> PlayersSoundClassBlueprint(TEXT("/Game/WildOmissionCore/Audio/Players"));
	if (PlayersSoundClassBlueprint.Succeeded())
	{
		PlayersSoundClass = PlayersSoundClassBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundClass> WeatherSoundClassBlueprint(TEXT("/Game/WildOmissionCore/Audio/Weather"));
	if (WeatherSoundClassBlueprint.Succeeded())
	{
		WeatherSoundClass = WeatherSoundClassBlueprint.Object;
	}
}

UWildOmissionGameInstance* UWildOmissionGameInstance::GetWildOmissionGameInstance(UWorld* WorldContextObject)
{
	if (WorldContextObject == nullptr)
	{
		return nullptr;
	}

	return Cast<UWildOmissionGameInstance>(WorldContextObject->GetGameInstance());
}

void UWildOmissionGameInstance::Init()
{
	Super::Init();
	
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UWildOmissionGameInstance::LoadedNewMap);

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem == nullptr)
	{
		return;
	}

	SessionInterface = OnlineSubsystem->GetSessionInterface();
	FriendsInterface = OnlineSubsystem->GetFriendsInterface();

	if (!SessionInterface.IsValid() || GEngine == 0)
	{
		return;
	}

	this->IsDedicatedServerInstance() ?
	SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UWildOmissionGameInstance::OnCreateDedicatedSessionComplete)
		: SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UWildOmissionGameInstance::OnCreateSessionComplete);
	//SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UWildOmissionGameInstance::OnDestroySessionComplete);
	SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UWildOmissionGameInstance::OnFindSessionsComplete);
	SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UWildOmissionGameInstance::OnJoinSessionComplete);
	GEngine->OnNetworkFailure().AddUObject(this, &UWildOmissionGameInstance::OnNetworkFailure);

	ServerAdministrators = NewObject<UServerAdministrators>(this);
	if (ServerAdministrators == nullptr)
	{
		UE_LOG(LogInit, Error, TEXT("Failed to create server administrators"))
			return;
	}

	ServerAdministrators->OnCreation();

	AchievementsManager = NewObject<UAchievementsManager>(this);
	if (AchievementsManager == nullptr)
	{
		UE_LOG(LogInit, Error, TEXT("Failed to create achievement manager."));
		return;
	}

	AchievementsManager->OnCreation();

	WorkshopManager = NewObject<UWorkshopManager>(this);
	if (WorkshopManager == nullptr)
	{
		UE_LOG(LogInit, Error, TEXT("Failed to create workshop manager."));
		return;
	}

	WorkshopManager->OnCreation();

	ApplyAudioSettings();
	RunAutoConfigQualitySettings();
}

void UWildOmissionGameInstance::Shutdown()
{
	Super::Shutdown();

	EndExistingSession();
}

void UWildOmissionGameInstance::ShowMainMenuWidget()
{
	if (MainMenuWidgetBlueprintClass == nullptr)
	{
		return;
	}

	MainMenuWidget = CreateWidget<UMainMenuWidget>(this, MainMenuWidgetBlueprintClass);
	if (MainMenuWidget == nullptr)
	{
		UE_LOG(LogPlayerController, Error, TEXT("Failed to create the main menu widget"))
		return;
	}

	MainMenuWidget->Setup(this);
	OnMainMenu = true;
}

void UWildOmissionGameInstance::ShowGameplayMenuWidget()
{
	if (GameplayMenuWidgetBlueprintClass == nullptr)
	{
		return;
	}

	if (IsValid(GameplayMenuWidget))
	{
		GameplayMenuWidget->Teardown();
		GameplayMenuWidget = nullptr;
	}

	GameplayMenuWidget = CreateWidget<UGameplayMenuWidget>(this, GameplayMenuWidgetBlueprintClass);

	if (GameplayMenuWidget == nullptr)
	{
		UE_LOG(LogPlayerController, Error, TEXT("Failed to create gameplay menu widget"));
		return;
	}

	GameplayMenuWidget->Show();
	GameplayMenuWidget->SetMenuInterface(this);
	GameplayMenuWidget->OnClosed.AddDynamic(this, &UWildOmissionGameInstance::ClearGameplayMenuWidget);

	UWorld* World = GetWorld();
	if (World && World->GetNetMode() == ENetMode::NM_Standalone)
	{
		UGameplayStatics::SetGamePaused(World, true);
	}
}

void UWildOmissionGameInstance::ClearGameplayMenuWidget()
{
	GameplayMenuWidget = nullptr;

	UWorld* World = GetWorld();
	if (World && World->GetNetMode() == ENetMode::NM_Standalone)
	{
		UGameplayStatics::SetGamePaused(World, false);
	}
}

void UWildOmissionGameInstance::RunAutoConfigQualitySettings(bool Override)
{
	UWildOmissionGameUserSettings* WOUserSettings = UWildOmissionGameUserSettings::GetWildOmissionGameUserSettings();
	if (WOUserSettings == nullptr || (WOUserSettings->GetHasRunAutoConfig() && !Override))
	{
		return;
	}

	WOUserSettings->RunHardwareBenchmark(10, 1.0f, 0.2f);
	WOUserSettings->ApplyHardwareBenchmarkResults();

	WOUserSettings->SetHasRunAutoConfig(true);
	WOUserSettings->ApplySettings(false);
}

void UWildOmissionGameInstance::StartLoading()
{
	Loading = true;

	if (LoadingMenuWidgetBlueprintClass == nullptr)
	{
		UE_LOG(LogLoad, Warning, TEXT("Something went wrong displaying the loading screen."));
		return;
	}

	if (LoadingMenuWidget)
	{
		LoadingMenuWidget->Teardown();
		LoadingMenuWidget = nullptr;
	}

	LoadingMenuWidget = CreateWidget<ULoadingMenuWidget>(this, LoadingMenuWidgetBlueprintClass);
	LoadingMenuWidget->Setup();
	LoadingMenuWidget->SetLoadingTitle(LoadingTitle);
	LoadingMenuWidget->SetLoadingStep(LoadingSubtitle);
}

void UWildOmissionGameInstance::StopLoading()
{
	Loading = false;
	if (LoadingMenuWidget == nullptr)
	{
		return;
	}

	LoadingMenuWidget->Teardown();
	LoadingMenuWidget = nullptr;
}

void UWildOmissionGameInstance::SetLoadingTitle(const FString& InLoadingTitle)
{
	LoadingTitle = InLoadingTitle;
	if (LoadingMenuWidget)
	{
		LoadingMenuWidget->SetLoadingTitle(LoadingTitle);
	}
}

void UWildOmissionGameInstance::SetLoadingSubtitle(const FString& InLoadingSubtitle)
{
	LoadingSubtitle = InLoadingSubtitle;
	if (LoadingMenuWidget)
	{
		LoadingMenuWidget->SetLoadingStep(InLoadingSubtitle);
	}
}

void UWildOmissionGameInstance::CreateWorld(const FString& WorldName, const FString& SeedOverride)
{
	const FString NewWorldInformationDirectory = WorldName + TEXT("/WorldInformation");
	const FString NewWorldDataDirectory = WorldName + TEXT("/WorldData");
	UWorldInformation* NewWorldInformation = Cast<UWorldInformation>(UGameplayStatics::CreateSaveGameObject(UWorldInformation::StaticClass()));
	UWorldData* NewWorldData = Cast<UWorldData>(UGameplayStatics::CreateSaveGameObject(UWorldData::StaticClass()));

	FDateTime Time;
	Time = FDateTime::Now();

	NewWorldInformation->LastPlayedTime = Time;

	NewWorldInformation->LevelFile = TEXT("LV_Procedural");
	NewWorldInformation->Version = UWorldInformation::GetCurrentVersion();

	NewWorldInformation->CreationInformation.Name = WorldName;
	NewWorldInformation->CreationInformation.Day = Time.GetDay();
	NewWorldInformation->CreationInformation.Month = Time.GetMonth();
	NewWorldInformation->CreationInformation.Year = Time.GetYear();

	const int32 MinSeed = 0;
	const int32 MaxSeed = 999999999;
	const uint32 RandomSeed = FMath::RandRange(MinSeed, MaxSeed);
	const uint32 CustomSeed = SeedOverride.IsNumeric() ? FCString::Atoi(*SeedOverride) : GetTypeHash(SeedOverride);
	const uint32 Seed = SeedOverride.IsEmpty() ? RandomSeed : CustomSeed;

	NewWorldInformation->Seed = Seed;

	UGameplayStatics::SaveGameToSlot(NewWorldInformation, NewWorldInformationDirectory, 0);
	UGameplayStatics::SaveGameToSlot(NewWorldData, NewWorldDataDirectory, 0);
}

bool UWildOmissionGameInstance::DoesWorldAlreadyExist(const FString& WorldName) const
{
	const TArray<FString> WorldNames = ASaveManager::GetAllWorldFolderNames();

	for (const FString& CurrentWorldName : WorldNames)
	{
		if (CurrentWorldName != WorldName)
		{
			continue;
		}

		return true;
	}

	return false;
}

void UWildOmissionGameInstance::RenameWorld(const FString& OldWorldName, const FString& NewWorldName)
{
	// Directory strings
	const FString OldWorldInformationDirectory = OldWorldName + TEXT("/WorldInformation");
	const FString OldWorldDataDirectory = OldWorldName + TEXT("/WorldData");
	const FString NewWorldInformationDirectory = NewWorldName + TEXT("/WorldInformation");
	const FString NewWorldDataDirectory = NewWorldName + TEXT("/WorldData");

	// Load the world with the old name
	UWorldInformation* RenamingWorldInformation = Cast<UWorldInformation>(UGameplayStatics::CreateSaveGameObject(UWorldInformation::StaticClass()));
	RenamingWorldInformation = Cast<UWorldInformation>(UGameplayStatics::LoadGameFromSlot(OldWorldInformationDirectory, 0));
	UWorldData* RenamingWorldData = Cast<UWorldData>(UGameplayStatics::CreateSaveGameObject(UWorldData::StaticClass()));
	RenamingWorldData = Cast<UWorldData>(UGameplayStatics::LoadGameFromSlot(OldWorldDataDirectory, 0));

	// Change world name in WorldInformation
	RenamingWorldInformation->CreationInformation.Name = NewWorldName;

	// Save world under new name
	UGameplayStatics::SaveGameToSlot(RenamingWorldInformation, NewWorldInformationDirectory, 0);
	UGameplayStatics::SaveGameToSlot(RenamingWorldData, NewWorldDataDirectory, 0);
	
	// TODO move world icon too

	// Delete the old save with the old name
	UGameplayStatics::DeleteGameInSlot(OldWorldInformationDirectory, 0);
	UGameplayStatics::DeleteGameInSlot(OldWorldDataDirectory, 0);
}

void UWildOmissionGameInstance::DeleteWorld(const FString& WorldName)
{
	UGameplayStatics::DeleteGameInSlot(WorldName, 0);
}

void UWildOmissionGameInstance::StartSession()
{
	if (SessionInterface.IsValid() == false)
	{
		UE_LOG(LogOnlineSession, Warning, TEXT("Couldn't start Session, SessionInterface not valid."));
		return;
	}

	UE_LOG(LogOnlineSession, Display, TEXT("Starting Session"));
	SessionInterface->StartSession(SESSION_NAME);
}

void UWildOmissionGameInstance::OpenLevel(const FString& Address)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	APlayerController* LocalPlayerController = World->GetFirstPlayerController();
	if (LocalPlayerController == nullptr)
	{
		return;
	}

	LocalPlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}

void UWildOmissionGameInstance::QuitToMenu()
{
	UE_LOG(LogPlayerController, Display, TEXT("Returning to main menu."));
	ReturnToMainMenu();

	EndExistingSession();
}

void UWildOmissionGameInstance::RefreshServerList(bool IsDedicated)
{
	UE_LOG(LogOnlineSession, Display, TEXT("Refreshing server list."));


	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (SessionSearch.IsValid() == false)
	{
		UE_LOG(LogOnlineSession, Warning, TEXT("Invalid Session Search"));
		return;
	}
	// Uncomment for lan results using null
	//SessionSearch->bIsLanQuery = true;
	SessionSearch->MaxSearchResults = 100;
	
	const FName SearchSetting = IsDedicated ? SEARCH_DEDICATED_ONLY : SEARCH_PRESENCE;

	SessionSearch->QuerySettings.Set(SearchSetting, true, EOnlineComparisonOp::Equals);
	
	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void UWildOmissionGameInstance::ApplyAudioSettings()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}
	
	UWildOmissionGameUserSettings* WOUserSettings = UWildOmissionGameUserSettings::GetWildOmissionGameUserSettings();
	if (WOUserSettings == nullptr)
	{
		return;
	}

	UGameplayStatics::SetSoundMixClassOverride(World, MasterSoundMixModifier, MasterSoundClass, WOUserSettings->GetMasterVolume(), 1.0f, 0.2f);
	UGameplayStatics::SetSoundMixClassOverride(World, MasterSoundMixModifier, MusicSoundClass, WOUserSettings->GetMusicVolume(), 1.0f, 0.2f);
	UGameplayStatics::SetSoundMixClassOverride(World, MasterSoundMixModifier, DeployablesSoundClass, WOUserSettings->GetDeployablesVolume(), 1.0f, 0.2f);
	UGameplayStatics::SetSoundMixClassOverride(World, MasterSoundMixModifier, EnvironmentSoundClass, WOUserSettings->GetEnvironmentVolume(), 1.0f, 0.2f);
	UGameplayStatics::SetSoundMixClassOverride(World, MasterSoundMixModifier, FriendlyCreaturesSoundClass, WOUserSettings->GetFriendlyCreaturesVolume(), 1.0f, 0.2f);
	UGameplayStatics::SetSoundMixClassOverride(World, MasterSoundMixModifier, HostileCreaturesSoundClass, WOUserSettings->GetHostileCreaturesVolume(), 1.0f, 0.2f);
	UGameplayStatics::SetSoundMixClassOverride(World, MasterSoundMixModifier, PlayersSoundClass, WOUserSettings->GetPlayersVolume(), 1.0f, 0.2f);
	UGameplayStatics::SetSoundMixClassOverride(World, MasterSoundMixModifier, WeatherSoundClass, WOUserSettings->GetWeatherVolume(), 1.0f, 0.2f);
	UGameplayStatics::PushSoundMixModifier(World, MasterSoundMixModifier);
}

//****************************
// HOSTING/JOINING
//****************************
void UWildOmissionGameInstance::StartSingleplayer(const FString& WorldName, const uint8& GameMode)
{
	if (MainMenuWidget == nullptr)
	{
		UE_LOG(LogPlayerController, Error, TEXT("Cannot start singleplayer, MainMenuWidget was nullptr."));
		return;
	}

	OnMainMenu = false;

	// Remove the menu from viewport
	MainMenuWidget->Teardown();

	// Show the loading menu
	StartLoading();
	SetLoadingTitle(TEXT("Loading Game"));
	SetLoadingSubtitle(TEXT("Loading level."));

	WorldToLoad = WorldName;
	GameModeIndex = GameMode;
	UE_LOG(LogTemp, Warning, TEXT("StartSingleplayer WorldName: %s"), *WorldName);
	FString WorldInformationDirectory = WorldToLoad + TEXT("/WorldInformation");

	UWorld* World = GetWorld();
	UWorldInformation* WorldInformation = Cast<UWorldInformation>(UGameplayStatics::LoadGameFromSlot(WorldInformationDirectory, 0));
	if (World == nullptr || WorldInformation== nullptr)
	{
		return;
	}

	const FString LevelFileString = WorldInformation->LevelFile;
	const FString GameModeString = FString::Printf(TEXT("%i"), GameModeIndex);
	const FString LoadString = FString::Printf(TEXT("/Game/WildOmissionCore/Levels/%s?worldname=%s?gamemode=%s"), *LevelFileString, *WorldName, *GameModeString);
	// Server travel to the game level
	World->ServerTravel(LoadString);
}

void UWildOmissionGameInstance::HostServer(const FString& ServerName, const FString& WorldName, bool FriendsOnly, const int32& MaxPlayerCount, const uint8& GameMode)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	DesiredServerName = ServerName;
	WorldToLoad = WorldName;
	FriendsOnlySession = FriendsOnly;
	DesiredMaxPlayerCount = MaxPlayerCount;
	GameModeIndex = GameMode;

	FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
	if (ExistingSession != nullptr)
	{
		FOnDestroySessionCompleteDelegate DestroySessionDelegate;
		DestroySessionDelegate.BindUObject(this, &UWildOmissionGameInstance::CreateSession);
		SessionInterface->DestroySession(SESSION_NAME, DestroySessionDelegate);
		return;
	}

	CreateSession();
}

void UWildOmissionGameInstance::JoinServer(const uint32& Index)
{
	if (!SessionInterface.IsValid() || !SessionSearch.IsValid() || MainMenuWidget == nullptr)
	{
		return;
	}

	EndExistingSession();

	MainMenuWidget->Teardown();

	SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[Index]);
	UE_LOG(LogOnlineSession, Display, TEXT("Joining Server Index: %i"), Index);
}

void UWildOmissionGameInstance::CreateSession(FName SessionName, bool Success)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = false;
	SessionSettings.NumPublicConnections = DesiredMaxPlayerCount;
	SessionSettings.bAllowInvites = true;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bAllowJoinViaPresence = true;
	SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;

	SessionSettings.Set(FRIENDS_ONLY_SETTINGS_KEY, FriendsOnlySession, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set(SERVER_NAME_SETTINGS_KEY, DesiredServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set(LEVEL_FILE_SETTINGS_KEY, WorldToLoad, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set(GAME_VERSION_SETTINGS_KEY, GameVersion, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);
}

void UWildOmissionGameInstance::LoadedNewMap(UWorld* InWorld)
{
	if (Loading)
	{
		StartLoading();
	}

	if (MainMenuWidget && !LastError.Acknowlaged)
	{
		MainMenuWidget->OpenErrorPrompt(LastError.Title, LastError.Message);
		LastError.Clear();
	}
}

void UWildOmissionGameInstance::EndExistingSession()
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
	if (ExistingSession == nullptr)
	{
		return;
	}

	SessionInterface->DestroySession(SESSION_NAME);
}

//****************************
// Subsystem Delegates
//****************************
void UWildOmissionGameInstance::OnCreateDedicatedSessionComplete(FName SessionName, bool Success)
{
	if (Success == false)
	{
		UE_LOG(LogOnlineSession, Error, TEXT("Could not create session %s"), *SessionName.ToString());
		return;
	}

	OnMainMenu = false;

	//StartSession();
	UE_LOG(LogOnlineSession, Display, TEXT("Finished setting up dedicated server session."));
}

void UWildOmissionGameInstance::OnCreateSessionComplete(FName SessionName, bool Success)
{
	if (Success == false || MainMenuWidget == nullptr)
	{
		UE_LOG(LogOnlineSession, Error, TEXT("Could not create session"));
		return;
	}

	OnMainMenu = false;

	// Remove the menu from viewport
	MainMenuWidget->Teardown();

	// Show the loading menu
	StartLoading();
	SetLoadingTitle(TEXT("Loading Game"));
	SetLoadingSubtitle(TEXT("Loading level."));

	FString WorldInformationDirectory = WorldToLoad + TEXT("/WorldInformation");

	UWorld* World = GetWorld();
	UWorldInformation* WorldInformation = Cast<UWorldInformation>(UGameplayStatics::LoadGameFromSlot(WorldInformationDirectory, 0));
	if (World == nullptr || WorldInformation == nullptr)
	{
		return;
	}

	const FString FriendsOnlyString = FString::Printf(TEXT("%i"), FriendsOnlySession);
	const FString LevelFileString = WorldInformation->LevelFile;
	const FString GameModeString = FString::Printf(TEXT("%i"), GameModeIndex);
	const FString LoadString = FString::Printf(TEXT("/Game/WildOmissionCore/Levels/%s?listen?worldname=%s?friendsonly=%s?gamemode=%s"),
		*LevelFileString, *WorldToLoad, *FriendsOnlyString, *GameModeString);
	// Server travel to the game level
	World->ServerTravel(LoadString);
}

void UWildOmissionGameInstance::OnDestroySessionComplete(FName SessionName, bool Success)
{
	if (Success == false)
	{
		return;
	}

	CreateSession();
}

void UWildOmissionGameInstance::OnFindSessionsComplete(bool Success)
{
	if (OnMainMenu == false || Success == false || SessionSearch.IsValid() == false || MainMenuWidget == nullptr)
	{
		UE_LOG(LogOnlineSession, Warning, TEXT("Found sessions but they are invalid"));
		return;
	}
	/*
	Setup the server list
	*/

	TArray<FServerData> ServerNames;
	for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
	{
		bool IsThisFriendsOnlySession = false;
		if (SearchResult.Session.SessionSettings.Get(FRIENDS_ONLY_SETTINGS_KEY, IsThisFriendsOnlySession))
		{
			const bool IsHostFriend = FriendsInterface->IsFriend(0, *SearchResult.Session.OwningUserId, TEXT("FriendsList"));
			if (IsThisFriendsOnlySession == true && IsHostFriend == false)
			{
				continue;
			}
		}
		else
		{
			continue;
		}

		FString HostGameVersion = TEXT("");
		if (SearchResult.Session.SessionSettings.Get(GAME_VERSION_SETTINGS_KEY, HostGameVersion))
		{
			if (!GetWorld()->IsEditorWorld() && GameVersion != HostGameVersion)
			{
				continue;
			}
		}
		else
		{
			continue;
		}

		FServerData Data;
		Data.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
		Data.CurrentPlayers = Data.MaxPlayers - SearchResult.Session.NumOpenPublicConnections;
		Data.HostUsername = SearchResult.Session.OwningUserName;
		Data.IsDedicated = SearchResult.Session.SessionSettings.bIsDedicated;
		Data.PingMS = SearchResult.PingInMs;

		FString ServerName;
		if (!Data.IsDedicated && SearchResult.Session.SessionSettings.Get(SERVER_NAME_SETTINGS_KEY, ServerName))
		{
			Data.Name = ServerName;
		}
		else if (Data.IsDedicated)
		{
			Data.Name = Data.HostUsername;
		}
		else
		{
			Data.Name = "Invalid Name";
		}

		ServerNames.Add(Data);
	}

	MainMenuWidget->SetServerList(ServerNames);
}

void UWildOmissionGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface.IsValid() == false)
	{
		return;
	}
	FString Address;
	if (!SessionInterface->GetResolvedConnectString(SessionName, Address))
	{
		UE_LOG(LogOnline, Error, TEXT("Could not resolve connect string"));
		QuitToMenu();
		return;
	}

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (PlayerController == nullptr)
	{
		QuitToMenu();
		return;
	}

	StartLoading();
	SetLoadingTitle(TEXT("Loading Game"));
	SetLoadingSubtitle(TEXT("Traveling to host."));

	PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}

void UWildOmissionGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	LastError.Set(TEXT("Network Failure"), ErrorString);
	QuitToMenu();
}

IOnlineFriendsPtr UWildOmissionGameInstance::GetFriendsInterface() const
{
	return FriendsInterface;
}

FName UWildOmissionGameInstance::GetSessionName()
{
	return SESSION_NAME;
}

FName UWildOmissionGameInstance::GetServerNameSettingsKey()
{
	return SERVER_NAME_SETTINGS_KEY;
}

FName UWildOmissionGameInstance::GetFriendsOnlySettingsKey()
{
	return FRIENDS_ONLY_SETTINGS_KEY;
}

FName UWildOmissionGameInstance::GetLevelFileSettingsKey()
{
	return LEVEL_FILE_SETTINGS_KEY;
}

FName UWildOmissionGameInstance::GetGameVersionSettingsKey()
{
	return GAME_VERSION_SETTINGS_KEY;
}

FName UWildOmissionGameInstance::GetSearchPresence()
{
	return SEARCH_PRESENCE;
}

FString UWildOmissionGameInstance::GetVersion() const
{
	return GameVersion;
}

UMainMenuWidget* UWildOmissionGameInstance::GetMainMenuWidget() const
{
	return MainMenuWidget;
}

UGameplayMenuWidget* UWildOmissionGameInstance::GetGameplayMenuWidget() const
{
	return GameplayMenuWidget;
}

FGameError::FGameError()
{
	Acknowlaged = true;
	Title = TEXT("");
	Message = TEXT("");
}

void FGameError::Set(const FString& ErrorTitle, const FString& ErrorMessage)
{
	Acknowlaged = false;
	Title = ErrorTitle;
	Message = ErrorMessage;
}

void FGameError::Clear()
{
	Acknowlaged = true;
	Title = TEXT("");
	Message = TEXT("");
}