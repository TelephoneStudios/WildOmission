// Copyright Telephone Studios. All Rights Reserved.


#include "UI/MainMenuWidget.h"
#include "SaveUpdater.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "PlayWorldSelectionWidget.h"
#include "UI/WorldCreationWidget.h"
#include "UI/WorldMenuWidget.h"
#include "UI/RenameWorldWidget.h"
#include "UI/DeleteWorldWidget.h"
#include "ServerBrowserWidget.h"
#include "SaveManager.h"
#include "UI/OptionsWidget.h"
#include "UI/CharacterCustomizationWidget.h"
#include "UI/WorkshopMenuWidget.h"
#include "CreditsWidget.h"
#include "ErrorMessagePrompt.h"
#include "SteamHelperFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Log.h"

const static TArray<FString> SPLASH_MESSAGES = {
	TEXT("Beta!"),
	TEXT("Aww Cute Poly Piggies <3"),
	TEXT("A Wild Storm Appears..."),
	TEXT("Now with crouching!"),
	TEXT("Larch was 16 at the start of development."),
	TEXT("Not possible without you!"),
	TEXT("Not a day off in 2023"),
	TEXT("Played by FloppySword!"),
	TEXT("NELSInfinity <3"),
	TEXT("rcjoe69 <3"),
	TEXT("dmyress <3"),
	TEXT("Violafame <3"),
	TEXT("TyceMcMac <3"),
	TEXT("Made *mostly* on a Model M Keyboard!"),
	TEXT("Refined Metal is obtained from zombies, harvesting metal nodes with a metal pickaxe, or by crafting"),
	TEXT("Made by Larch"),
	TEXT("Build without limits!"),
	TEXT("Build a massive tower!"),
	TEXT("This is a splash text."),
	TEXT("96.7% C++"),
	TEXT("1.8% C"),
	TEXT("1.5% C#"),
	TEXT("Updated Regularly!"),
	TEXT("3,000,000+ Lines Of Code."),
	TEXT("Also play DeadPoly!"),
	TEXT("Now Open Source!"),
	TEXT("Free!")

};

UMainMenuWidget::UMainMenuWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	SetIsFocusable(true);

	MenuSwitcher = nullptr;

	PlayButton = nullptr;
	OptionsButton = nullptr;
	CharacterCustomizationButton = nullptr;
	WorkshopButton = nullptr;
	FeedbackButton = nullptr;
	CreditsButton = nullptr;
	ExitButton = nullptr;
	
	SplashTextBlock = nullptr;

	MainMenuPanel = nullptr;
	MainMenu = nullptr;

	UpdatingWorldsMenu = nullptr;

	WorldSelectionMenuPanel = nullptr;
	PlayWorldSelectionMenu = nullptr;
	WorldCreationMenu = nullptr;
	WorldMenuPanel = nullptr;
	WorldMenu = nullptr;
	RenameWorldMenu = nullptr;
	DeleteWorldMenu = nullptr;
	ServerBrowserMenuPanel = nullptr;
	ServerBrowserMenu = nullptr;
	OptionsMenuPanel = nullptr;
	OptionsMenu = nullptr;
	CharacterCustomizationMenu = nullptr;
	WorkshopMenu = nullptr;
	CreditsMenuPanel = nullptr;
	CreditsMenu = nullptr;
	ErrorMessagePrompt = nullptr;
	MenuInterface = nullptr;
}

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PlayButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OpenPlayWorldSelectionMenu);
	OptionsButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OpenOptionsMenu);
	CharacterCustomizationButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OpenCharacterCustomizationMenu);
	WorkshopButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OpenWorkshopMenu);
	FeedbackButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OpenFeedbackPage);
	CreditsButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OpenCreditsMenu);
	ExitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::ExitGame);

	RefreshSplashText();

	PlayWorldSelectionMenu->OnSelectButtonClicked.AddDynamic(this, &UMainMenuWidget::OpenWorldMenu);
	PlayWorldSelectionMenu->OnCreateNewWorldButtonClicked.AddDynamic(this, &UMainMenuWidget::OpenWorldCreationMenu);
	PlayWorldSelectionMenu->OnMultiplayerButtonClicked.AddDynamic(this, &UMainMenuWidget::OpenServerBrowserMenu);
	PlayWorldSelectionMenu->OnCancelButtonClicked.AddDynamic(this, &UMainMenuWidget::OpenMainMenu);

	WorldCreationMenu->OnOpenWorldMenuRequested.AddDynamic(this, &UMainMenuWidget::OpenWorldMenuForWorld);
	WorldCreationMenu->OnCancelButtonClicked.AddDynamic(this, &UMainMenuWidget::OpenPlayWorldSelectionMenu);

	WorldMenu->OnPlayButtonClicked.AddDynamic(this, &UMainMenuWidget::HostGame);
	WorldMenu->OnRenameButtonClicked.AddDynamic(this, &UMainMenuWidget::OpenRenameWorldMenu);
	WorldMenu->OnDeleteButtonClicked.AddDynamic(this, &UMainMenuWidget::OpenDeleteWorldMenu);
	WorldMenu->OnCancelButtonClicked.AddDynamic(this, &UMainMenuWidget::OpenPlayWorldSelectionMenu);

	RenameWorldMenu->OnRenameButtonClicked.AddDynamic(this, &UMainMenuWidget::OpenWorldMenuForWorld);
	RenameWorldMenu->OnCancelButtonClicked.AddDynamic(this, &UMainMenuWidget::OpenWorldMenu);

	DeleteWorldMenu->OnDeleteButtonClicked.AddDynamic(this, &UMainMenuWidget::OpenPlayWorldSelectionMenu);
	DeleteWorldMenu->OnCancelButtonClicked.AddDynamic(this, &UMainMenuWidget::OpenWorldMenu);

	ServerBrowserMenu->OnJoinButtonClicked.AddDynamic(this, &UMainMenuWidget::JoinServer);
	ServerBrowserMenu->OnRefreshButtonClicked.AddDynamic(this, &UMainMenuWidget::RefreshServerList);
	ServerBrowserMenu->OnCancelButtonClicked.AddDynamic(this, &UMainMenuWidget::OpenPlayWorldSelectionMenu);

	OptionsMenu->OnBackButtonClicked.AddDynamic(this, &UMainMenuWidget::OpenMainMenu);

	CharacterCustomizationMenu->OnBackButtonClicked.AddDynamic(this, &UMainMenuWidget::OpenMainMenu);

	WorkshopMenu->OnBackButtonClicked.AddDynamic(this, &UMainMenuWidget::OpenMainMenu);

	CreditsMenu->OnBackButtonClicked.AddDynamic(this, &UMainMenuWidget::OpenMainMenu);

	ErrorMessagePrompt->OnCloseButtonClicked.AddDynamic(this, &UMainMenuWidget::OpenMainMenu);
}

void UMainMenuWidget::SetServerList(TArray<FServerData> InServerData)
{
	ServerBrowserMenu->SetServerList(InServerData);
}

void UMainMenuWidget::Setup(IMenuInterface* InMenuInterface)
{
	this->AddToViewport();
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
	{
		FInputModeUIOnly InputModeData;
		InputModeData.SetWidgetToFocus(this->TakeWidget());
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputModeData);
		PlayerController->bShowMouseCursor = true;
	}

	MenuInterface = InMenuInterface;

	// do any world/workshop processing on separate thread so game doesn't lock up
	MenuSwitcher->SetActiveWidget(UpdatingWorldsMenu);
	Async(EAsyncExecution::Thread, [this]()
		{
			TArray<FString> OldWorldNames = ASaveManager::GetAllWorldNamesV1();
			if (!OldWorldNames.IsEmpty())
			{
				USaveUpdater::UpdateWorldFiles(OldWorldNames);
			}
			
			AsyncTask(ENamedThreads::GameThread, [this]()
				{
					MenuSwitcher->SetActiveWidget(InstallingWorkshopWorldsMenu);
				});

			Async(EAsyncExecution::Thread, [this]()
				{
					UWorkshopManager* WorkshopManager = UWorkshopManager::GetWorkshopManager();
					if (WorkshopManager)
					{
						WorkshopManager->CheckAndCopyNewWorkshopItems();
					}
					AsyncTask(ENamedThreads::GameThread, [this]()
						{
							OpenMainMenu();
						});
				});
		});

	
}

void UMainMenuWidget::Teardown()
{
	this->RemoveFromParent();
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
	{
		FInputModeGameOnly InputModeData;
		PlayerController->SetInputMode(InputModeData);
		PlayerController->bShowMouseCursor = false;
	}
}

IMenuInterface* UMainMenuWidget::GetMenuInterface() const
{
	return MenuInterface;
}

void UMainMenuWidget::RefreshSplashText()
{
	const int32 Index = FMath::RandRange(0, SPLASH_MESSAGES.Num() - 1);
	if (!SPLASH_MESSAGES.IsValidIndex(Index))
	{
		return;
	}
	
	FString SplashText = SPLASH_MESSAGES[Index];

	FDateTime TimeNow = FDateTime::Now();
	if (TimeNow.GetMonth() == 1 && TimeNow.GetDay() == 1)
	{
		SplashText = TEXT("Happy New Year!");
	}
	else if (TimeNow.GetMonth() == 1 && TimeNow.GetDay() == 13)
	{
		SplashText = TEXT("Happy Birthday Larch!");
	}
	else if (TimeNow.GetMonth() == 4 && TimeNow.GetDay() == 1)
	{
		SplashText = TEXT("April Fools!");
	}
	else if (TimeNow.GetMonth() == 5 && TimeNow.GetMonth() == 12)
	{
		SplashText = TEXT("Happy Birthday Tyce!");
	}
	else if (TimeNow.GetMonth() == 10 && TimeNow.GetDay() == 31)
	{
		SplashText = TEXT("Happy Halloween!");
	}
	else if (TimeNow.GetMonth() == 12 && (TimeNow.GetDay() == 24 || TimeNow.GetDay() == 25))
	{
		SplashText = TEXT("Merry XMas!");
	}
	else if (TimeNow.GetHour12() == 3 && TimeNow.GetMinute() == 33)
	{
		SplashText = TEXT("333");
	}
	else if (TimeNow.GetMinute() == 13)
	{
		SplashText = TEXT("13");
	}

	SplashTextBlock->SetText(FText::FromString(SplashText));
}

void UMainMenuWidget::JoinServer(const uint32& ServerIndex)
{
	if (MenuInterface == nullptr)
	{
		return;
	}

	MenuInterface->JoinServer(ServerIndex);
}

void UMainMenuWidget::RefreshServerList(bool IsDedicated)
{
	if (MenuInterface == nullptr)
	{
		return;
	}

	MenuInterface->RefreshServerList(IsDedicated);
}

void UMainMenuWidget::OpenMainMenu()
{
	if (MenuSwitcher == nullptr || MainMenuPanel == nullptr || UpdatingWorldsMenu == nullptr)
	{
		UE_LOG(LogMenuSystem, Warning, TEXT("Failed to switch to main menu"));
		return;
	}

	UE_LOG(LogMenuSystem, Verbose, TEXT("Switching to main menu"));
	MenuSwitcher->SetActiveWidget(MainMenuPanel);
}

void UMainMenuWidget::OpenPlayWorldSelectionMenu()
{
	if (MenuSwitcher == nullptr || WorldSelectionMenuPanel == nullptr 
		|| PlayWorldSelectionMenu == nullptr || MenuInterface == nullptr)
	{
		return;
	}

	PlayWorldSelectionMenu->SetWorldList(ASaveManager::GetAllWorldFolderNames());
	MenuSwitcher->SetActiveWidget(WorldSelectionMenuPanel);
}

void UMainMenuWidget::OpenWorldCreationMenu()
{
	if (MenuSwitcher == nullptr || WorldCreationMenu == nullptr)
	{
		return;
	}

	MenuSwitcher->SetActiveWidget(WorldCreationMenu);
}

void UMainMenuWidget::OpenWorldMenu()
{
	if (MenuSwitcher == nullptr || WorldMenu == nullptr || PlayWorldSelectionMenu->SelectedWorldName.IsSet() == false)
	{
		return;
	}

	OpenWorldMenuForWorld(PlayWorldSelectionMenu->SelectedWorldName.GetValue());
}

void UMainMenuWidget::OpenWorldMenuForWorld(const FString& WorldName)
{
	if (MenuSwitcher == nullptr || WorldMenuPanel == nullptr
		|| WorldMenu == nullptr)
	{
		return;
	}

	MenuSwitcher->SetActiveWidget(WorldMenuPanel);
	WorldMenu->Open(WorldName);
}

void UMainMenuWidget::HostGame(const FString& WorldName, const FString& ServerName, const bool IsMultiplayer, const bool IsFriendsOnly, const uint8& GameMode, const int32& MaxPlayerCount)
{
	if (MenuInterface == nullptr || WorldName.IsEmpty())
	{
		return;
	}

	UE_LOG(LogMenuSystem, Display, TEXT("Host game, World Name is %s"), *WorldName);
	IsMultiplayer ? MenuInterface->HostServer(ServerName, WorldName, IsFriendsOnly, MaxPlayerCount, GameMode) : MenuInterface->StartSingleplayer(WorldName, GameMode);
}

void UMainMenuWidget::OpenRenameWorldMenu()
{
	if (MenuSwitcher == nullptr || RenameWorldMenu == nullptr)
	{
		return;
	}

	MenuSwitcher->SetActiveWidget(RenameWorldMenu);
	RenameWorldMenu->Open(PlayWorldSelectionMenu->SelectedWorldName.GetValue());
}

void UMainMenuWidget::OpenDeleteWorldMenu()
{
	if (MenuSwitcher == nullptr || DeleteWorldMenu == nullptr)
	{
		return;
	}

	MenuSwitcher->SetActiveWidget(DeleteWorldMenu);
	DeleteWorldMenu->Open(PlayWorldSelectionMenu->SelectedWorldName.GetValue());
}

void UMainMenuWidget::OpenServerBrowserMenu()
{
	if (MenuSwitcher == nullptr || ServerBrowserMenuPanel == nullptr
		|| ServerBrowserMenu == nullptr)
	{
		return;
	}

	MenuSwitcher->SetActiveWidget(ServerBrowserMenuPanel);
	ServerBrowserMenu->OnOpen();
}

void UMainMenuWidget::OpenOptionsMenu()
{
	if (MenuSwitcher == nullptr || OptionsMenuPanel == nullptr
		|| OptionsMenu == nullptr)
	{
		return;
	}

	MenuSwitcher->SetActiveWidget(OptionsMenuPanel);
	OptionsMenu->Refresh();
}

void UMainMenuWidget::OpenCharacterCustomizationMenu()
{
	if (MenuSwitcher == nullptr || CharacterCustomizationMenu == nullptr)
	{
		return;
	}
	CharacterCustomizationMenu->OnOpen();
	MenuSwitcher->SetActiveWidget(CharacterCustomizationMenu);
}

void UMainMenuWidget::OpenWorkshopMenu()
{
	if (MenuSwitcher == nullptr || WorkshopMenu == nullptr)
	{
		return;
	}

	WorkshopMenu->OnOpen();
	MenuSwitcher->SetActiveWidget(WorkshopMenu);
}

void UMainMenuWidget::OpenFeedbackPage()
{
	const FString URL = TEXT("https://steamcommunity.com/app/2348700/discussions/3/");
	USteamHelperFunctionLibrary::OpenWebPageInOverlay(URL);
}

void UMainMenuWidget::OpenCreditsMenu()
{
	if (MenuSwitcher == nullptr || CreditsMenuPanel == nullptr 
		|| CreditsMenu == nullptr)
	{
		return;
	}

	MenuSwitcher->SetActiveWidget(CreditsMenuPanel);
}

void UMainMenuWidget::ExitGame()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (PlayerController == nullptr)
	{
		return;
	}

	PlayerController->ConsoleCommand(TEXT("quit"));
}

void UMainMenuWidget::OpenErrorPrompt(const FString& Title, const FString& Error)
{
	if (ErrorMessagePrompt == nullptr || MenuSwitcher == nullptr)
	{
		UE_LOG(LogMenuSystem, Warning, TEXT("MainMenuWidget::OpenErrorPrompt, ErrorMessagePrompt or MenuSwitcher was nullptr."));
		return;
	}

	MenuSwitcher->SetActiveWidget(ErrorMessagePrompt);
	ErrorMessagePrompt->SetMessage(Title, Error);
}
