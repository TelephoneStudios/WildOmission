// Copyright Telephone Studios. All Rights Reserved.


#include "UI/WorldCreationWidget.h"
#include "SaveManager.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Interfaces/GameSaveLoadController.h"
#include "AchievementsManager.h"
#include "Log.h"

UWorldCreationWidget::UWorldCreationWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	CreateWorldButton = nullptr;
	CancelButton = nullptr;
	WorldNameInputBox = nullptr;
	WorldSeedInputBox = nullptr;

	InvalidWarningBorder = nullptr;
	InvalidWarningTextBlock = nullptr;	
}

void UWorldCreationWidget::NativeConstruct()
{
	Super::NativeConstruct();

	HideInvalidWarning();

	CreateWorldButton->OnClicked.AddDynamic(this, &UWorldCreationWidget::CreateWorld);
	CreateWorldButton->SetIsEnabled(false);
	CancelButton->OnClicked.AddDynamic(this, &UWorldCreationWidget::BroadcastOnCancelButtonClicked);
	WorldNameInputBox->OnTextChanged.AddDynamic(this, &UWorldCreationWidget::WorldNameOnTextChanged);
}

void UWorldCreationWidget::ShowInvalidWarning(const FString& Warning)
{
	InvalidWarningTextBlock->SetText(FText::FromString(Warning));

	InvalidWarningBorder->SetVisibility(ESlateVisibility::Visible);
}

void UWorldCreationWidget::HideInvalidWarning()
{
	InvalidWarningBorder->SetVisibility(ESlateVisibility::Collapsed);
}

bool UWorldCreationWidget::WorldOfSameNameAlreadyExists(const FString& WorldName)
{
	return ASaveManager::WorldAlreadyExists(WorldName);
}

bool UWorldCreationWidget::WorldContainsInvalidCharacter(const FString& WorldName)
{
	TArray<FString> InvalidCharacters;
	InvalidCharacters.Add(TEXT("<"));
	InvalidCharacters.Add(TEXT(">"));
	InvalidCharacters.Add(TEXT(":"));
	InvalidCharacters.Add(TEXT("\""));
	InvalidCharacters.Add(TEXT("/"));
	InvalidCharacters.Add(TEXT("\\"));
	InvalidCharacters.Add(TEXT("|"));
	InvalidCharacters.Add(TEXT("?"));
	InvalidCharacters.Add(TEXT("*"));

	for (const FString& InvalidCharacter : InvalidCharacters)
	{
		if (!WorldName.Contains(InvalidCharacter))
		{
			continue;
		}

		return true;
	}

	TArray<FString> InvalidNames;
	InvalidNames.Add(TEXT("CON"));
	InvalidNames.Add(TEXT("PRN"));
	InvalidNames.Add(TEXT("AUX"));
	InvalidNames.Add(TEXT("NUL"));
	InvalidNames.Add(TEXT("COM0"));
	InvalidNames.Add(TEXT("COM1"));
	InvalidNames.Add(TEXT("COM2"));
	InvalidNames.Add(TEXT("COM3"));
	InvalidNames.Add(TEXT("COM4"));
	InvalidNames.Add(TEXT("COM5"));
	InvalidNames.Add(TEXT("COM6"));
	InvalidNames.Add(TEXT("COM7"));
	InvalidNames.Add(TEXT("COM8"));
	InvalidNames.Add(TEXT("COM9"));

	for (const FString& InvalidName : InvalidNames)
	{
		if (WorldName.ToLower() != InvalidName.ToLower())
		{
			continue;
		}

		return true;
	}

	return false;
}

void UWorldCreationWidget::CreateWorld()
{
	// Get the name of the save
	const FString NewWorldName = WorldNameInputBox->GetText().ToString();
	const FString NewWorldSeed = WorldSeedInputBox->GetText().ToString();

	if (NewWorldName.IsEmpty() || WorldOfSameNameAlreadyExists(NewWorldName) || WorldContainsInvalidCharacter(NewWorldName))
	{
		UE_LOG(LogSaveSystem, Warning, TEXT("World name error."));
		return;
	}

	IGameSaveLoadController* GameSaveLoadController = Cast<IGameSaveLoadController>(GetGameInstance());
	if (GameSaveLoadController == nullptr)
	{
		return;
	}

	// Create a new world with that name
	GameSaveLoadController->CreateWorld(NewWorldName, NewWorldSeed);

	// To World Menu
	if (!OnOpenWorldMenuRequested.IsBound())
	{
		return;
	}

	OnOpenWorldMenuRequested.Broadcast(NewWorldName);

	UAchievementsManager* AchievementsManager = UAchievementsManager::GetAchievementsManager();
	if (AchievementsManager == nullptr)
	{
		return;
	}

	AchievementsManager->UnlockAchievement(TEXT("ACH_INFINITE_POSSIBILITIES"));
}

void UWorldCreationWidget::BroadcastOnCancelButtonClicked()
{
	if (!OnCancelButtonClicked.IsBound())
	{
		return;
	}

	OnCancelButtonClicked.Broadcast();
}

void UWorldCreationWidget::WorldNameOnTextChanged(const FText& Text)
{
	FString TextString = Text.ToString();

	if (TextString.Len() > 32)
	{
		TextString = TextString.LeftChop(1);
	}

	WorldNameInputBox->SetText(FText::FromString(TextString));

	if (TextString.IsEmpty())
	{
		CreateWorldButton->SetIsEnabled(false);
	}

	if (WorldOfSameNameAlreadyExists(TextString))
	{
		CreateWorldButton->SetIsEnabled(false);
		ShowInvalidWarning(TEXT("World of same name already exists!"));
	}
	else if (WorldContainsInvalidCharacter(TextString))
	{
		CreateWorldButton->SetIsEnabled(false);
		ShowInvalidWarning(TEXT("World name contains an invalid character!"));
	}
	else
	{
		CreateWorldButton->SetIsEnabled(true);
		HideInvalidWarning();
	}
}
