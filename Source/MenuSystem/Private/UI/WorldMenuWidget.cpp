// Copyright Telephone Studios. All Rights Reserved.


#include "WorldMenuWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "OptionBoxes/MultiOptionBox.h"
#include "OptionBoxes/CheckOptionBox.h"
#include "OptionBoxes/SliderOptionBox.h"
#include "Components/TextBlock.h"
#include "Enums/GameDifficulty.h"
#include "Kismet/GameplayStatics.h"
#include "WorldInformation.h"
#include "SteamHelperFunctionLibrary.h"
#include "GameFramework/PlayerState.h"
#include "Log.h"

static const int32 CreativeModeAppID = 4877470;
static bool HasCreativeMode = false;

UWorldMenuWidget::UWorldMenuWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	Title = nullptr;
	PlayButton = nullptr;
	PlayButtonTextBlock = nullptr;
	RenameButton = nullptr;
	DeleteButton = nullptr;
	CancelButton = nullptr;
	DifficultyMultiOptionBox = nullptr;
	GameModeMultiOptionBox = nullptr;
	SeedTextBlock = nullptr;
	MultiplayerCheckOptionBox = nullptr;
	FriendsOnlyCheckOptionBox = nullptr;
	MaxPlayersSliderOptionBox = nullptr;
	HostSettingsMenu = nullptr;
	ServerNameInputBox = nullptr;

	WorldName = TEXT("");
}

void UWorldMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	DifficultyMultiOptionBox->AddOption(TEXT("Peaceful"));
	DifficultyMultiOptionBox->AddOption(TEXT("Easy"));
	DifficultyMultiOptionBox->AddOption(TEXT("Normal"));
	DifficultyMultiOptionBox->AddOption(TEXT("Hard"));

	// In order to play creative mode they must purchase the creative mode dlc
	GameModeMultiOptionBox->AddOption(TEXT("Survival"));
	GameModeMultiOptionBox->AddOption(TEXT("Creative"));

	MaxPlayersSliderOptionBox->SetMaxValue(16);
	MaxPlayersSliderOptionBox->SetMinValue(2);
	MaxPlayersSliderOptionBox->SetValue(8);

	PlayButton->OnClicked.AddDynamic(this, &UWorldMenuWidget::BroadcastPlayButtonClicked);
	RenameButton->OnClicked.AddDynamic(this, &UWorldMenuWidget::BroadcastRenameButtonClicked);
	DeleteButton->OnClicked.AddDynamic(this, &UWorldMenuWidget::BroadcastDeleteButtonClicked);
	CancelButton->OnClicked.AddDynamic(this, &UWorldMenuWidget::BroadcastCancelButtonClicked);
	MultiplayerCheckOptionBox->OnValueChanged.AddDynamic(this, &UWorldMenuWidget::MultiplayerCheckboxChanged);
	ServerNameInputBox->OnTextChanged.AddDynamic(this, &UWorldMenuWidget::ServerNameOnTextChanged);
	GameModeMultiOptionBox->OnValueChanged.AddDynamic(this, &UWorldMenuWidget::OnGameModeSelectionChanged);
}

void UWorldMenuWidget::Open(const FString& InWorldName)
{
	FString InformationDirectory = InWorldName + TEXT("/WorldInformation");
	WorldName = InWorldName;
	UWorldInformation* WorldInformation = Cast<UWorldInformation>(UGameplayStatics::CreateSaveGameObject(UWorldInformation::StaticClass()));

	WorldInformation= Cast<UWorldInformation>(UGameplayStatics::LoadGameFromSlot(InformationDirectory, 0));
	if (WorldInformation == nullptr)
	{
		UE_LOG(LogMenuSystem, Warning, TEXT("Failed to open world menu, WorldInformation is nullptr."));
		return;
	}

	Title->SetText(FText::FromString(WorldName));

	// Get Save File and select options
	DifficultyMultiOptionBox->SetSelectedIndex(WorldInformation->Difficulty.GetIntValue());

	HasCreativeMode = USteamHelperFunctionLibrary::IsDLCInstalled(CreativeModeAppID);

	if (HasCreativeMode)
	{
		// load game mode normally
		GameModeMultiOptionBox->SetSelectedIndex(WorldInformation->GameMode);
	}
	else
	{
		// if we don't have creative make it set to survival
		GameModeMultiOptionBox->SetSelectedIndex(0);
	}

	// Set the seed text block
	SeedTextBlock->SetText(FText::FromString(FString::Printf(TEXT("Seed: %i"), WorldInformation->Seed)));

	// Set Placeholder Server Name
	FString PlaceholderServerName;
	APlayerState* PlayerState = GetOwningPlayerState();
	if (PlayerState == nullptr)
	{
		return;
	}
	PlaceholderServerName = FString::Printf(TEXT("%s's Server"), *PlayerState->GetPlayerName());
	ServerNameInputBox->SetText(FText::FromString(PlaceholderServerName));

	const int32 WorldVersion = WorldInformation->Version;

	// If this is an old world, prevent it from being played
	if (WorldVersion < 2)
	{
		PlayButton->SetIsEnabled(false);
		PlayButtonTextBlock->SetColorAndOpacity(FSlateColor(FColor::Red));
		PlayButtonTextBlock->SetText(FText::FromString(TEXT("This world can only be played on an older version of Wild Omission")));
	}
	else if (WorldVersion > UWorldInformation::GetCurrentVersion())
	{
		PlayButton->SetIsEnabled(false);
		PlayButtonTextBlock->SetColorAndOpacity(FSlateColor(FColor::Red));
		PlayButtonTextBlock->SetText(FText::FromString(TEXT("This world can only be played on a newer version of Wild Omission")));
	}
}

int32 UWorldMenuWidget::GetWorldVersion() const
{
	// Get the save file
	FString InformationDirectory = WorldName + TEXT("/WorldInformation");
	UWorldInformation* WorldInformation = Cast<UWorldInformation>(UGameplayStatics::LoadGameFromSlot(InformationDirectory, 0));
	if (WorldInformation == nullptr)
	{

		UE_LOG(LogMenuSystem, Warning, TEXT("Failed to get world infomation in UWorldMenuWidget::GetWorldVersion"));
		return -1;
	}

	// Return the version
	return WorldInformation->Version;
}

TEnumAsByte<EGameDifficulty> UWorldMenuWidget::GetWorldDifficulty() const
{
	// Get the save file
	FString InformationDirectory = WorldName + TEXT("/WorldInformation");
	UWorldInformation* WorldInformation = Cast<UWorldInformation>(UGameplayStatics::LoadGameFromSlot(InformationDirectory, 0));
	if (WorldInformation == nullptr)
	{
		return EGameDifficulty::EGD_Normal;
	}

	// Return the save difficulty value
	return WorldInformation->Difficulty;
}

void UWorldMenuWidget::SetWorldDifficultyAndGameMode(const TEnumAsByte<EGameDifficulty>& NewDifficulty, const uint8& NewGameMode)
{
	// Get the save file
	FString InformationDirectory = WorldName + TEXT("/WorldInformation");
	UWorldInformation* WorldInformation = Cast<UWorldInformation>(UGameplayStatics::LoadGameFromSlot(InformationDirectory, 0));
	if (WorldInformation == nullptr)
	{
		return;
	}

	// Set the difficulty value to NewDifficulty
	WorldInformation->Difficulty = NewDifficulty;
	
	WorldInformation->GameMode = NewGameMode;

	// Save the save game
	UGameplayStatics::SaveGameToSlot(WorldInformation, InformationDirectory, 0);
}

void UWorldMenuWidget::ServerNameOnTextChanged(const FText& Text)
{
	FString TextString = Text.ToString();

	if (TextString.Len() > 32)
	{
		TextString = TextString.LeftChop(1);
	}

	ServerNameInputBox->SetText(FText::FromString(TextString));
}

// This function is handling making sure that the user must have the creative DLC 
// in order to start a creative world
void UWorldMenuWidget::OnGameModeSelectionChanged(const FString& NewValue)
{
	if (NewValue != TEXT("Creative"))
	{
		// Make sure play button is enabled
		PlayButton->SetIsEnabled(true);
		PlayButtonTextBlock->SetColorAndOpacity(FSlateColor(FColor::White));
		PlayButtonTextBlock->SetText(FText::FromString(TEXT("Play")));
		return;
	}

	// If creative was selected and we don't have creative expansion open the store
	if (!HasCreativeMode)
	{
		USteamHelperFunctionLibrary::OpenStore(CreativeModeAppID);
		
		// Disable and explain why they can't play
		PlayButton->SetIsEnabled(false);
		PlayButtonTextBlock->SetColorAndOpacity(FSlateColor(FColor::Red));
		PlayButtonTextBlock->SetText(FText::FromString(TEXT("Creative Mode DLC required in order to play creative!")));
	}
}

void UWorldMenuWidget::MultiplayerCheckboxChanged(bool bIsChecked)
{
	HostSettingsMenu->SetIsEnabled(bIsChecked);
}

void UWorldMenuWidget::BroadcastPlayButtonClicked()
{
	if (!OnPlayButtonClicked.IsBound() 
		|| WorldName.IsEmpty())
	{
		return;
	}
	
	uint8 GameMode = GameModeMultiOptionBox->GetSelectedIndex();
	// force survival if creative mode isn't owned
	if (!HasCreativeMode)
	{
		GameMode = 0;
	}
	
	// Set Difficulty in save file
	SetWorldDifficultyAndGameMode(TEnumAsByte<EGameDifficulty>(DifficultyMultiOptionBox->GetSelectedIndex()), GameMode);

	const FString ServerName = ServerNameInputBox->GetText().ToString();
	const bool IsMultiplayer = MultiplayerCheckOptionBox->IsChecked();
	const bool IsFriendsOnly = FriendsOnlyCheckOptionBox->IsChecked();

	OnPlayButtonClicked.Broadcast(WorldName, ServerName, IsMultiplayer, IsFriendsOnly, GameMode, FMath::RoundToInt32(MaxPlayersSliderOptionBox->GetValue()));
}

void UWorldMenuWidget::BroadcastRenameButtonClicked()
{
	if (!OnRenameButtonClicked.IsBound())
	{
		return;
	}

	OnRenameButtonClicked.Broadcast();
}

void UWorldMenuWidget::BroadcastDeleteButtonClicked()
{
	if (!OnDeleteButtonClicked.IsBound())
	{
		return;
	}

	OnDeleteButtonClicked.Broadcast();
}

void UWorldMenuWidget::BroadcastCancelButtonClicked()
{
	if (!OnCancelButtonClicked.IsBound())
	{
		return;
	}

	OnCancelButtonClicked.Broadcast();
}