// Copyright Telephone Studios. All Rights Reserved.


#include "UI/RenameWorldWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "SaveManager.h"
#include "Interfaces/GameSaveLoadController.h"
#include "Log.h"

URenameWorldWidget::URenameWorldWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	WorldNameInputBox = nullptr;
	RenameButton = nullptr;
	CancelButton = nullptr;

	World = TEXT("");
}

void URenameWorldWidget::NativeConstruct()
{
	Super::NativeConstruct();

	HideInvalidWarning();

	WorldNameInputBox->OnTextChanged.AddDynamic(this, &URenameWorldWidget::WorldNameOnTextChanged);
	RenameButton->OnClicked.AddDynamic(this, &URenameWorldWidget::RenameWorld);
	RenameButton->SetIsEnabled(false);
	CancelButton->OnClicked.AddDynamic(this, &URenameWorldWidget::BroadcastCancelButtonClicked);
}

void URenameWorldWidget::Open(const FString& InWorld)
{
	World = InWorld;
	WorldNameInputBox->SetText(FText::FromString(InWorld));
	WorldNameOnTextChanged(WorldNameInputBox->GetText());
}

void URenameWorldWidget::RenameWorld()
{
	const FString NewWorldName = WorldNameInputBox->GetText().ToString();
	
	if (NewWorldName.IsEmpty() || WorldOfSameNameAlreadyExists(NewWorldName) || WorldContainsInvalidCharacter(NewWorldName))
	{
		UE_LOG(LogSaveSystem, Warning, TEXT("World name error."));
		return;
	}
	
	// Rename world
	ASaveManager::RenameWorld(World, NewWorldName);

	BroadcastRenameButtonClicked(NewWorldName);
}

void URenameWorldWidget::ShowInvalidWarning(const FString& Warning)
{
	InvalidWarningTextBlock->SetText(FText::FromString(Warning));
	InvalidWarningBorder->SetVisibility(ESlateVisibility::Visible);
}

void URenameWorldWidget::HideInvalidWarning()
{
	InvalidWarningBorder->SetVisibility(ESlateVisibility::Collapsed);
}

bool URenameWorldWidget::WorldOfSameNameAlreadyExists(const FString& WorldName)
{
	return ASaveManager::WorldAlreadyExists(WorldName);
}

bool URenameWorldWidget::WorldContainsInvalidCharacter(const FString& WorldName)
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

void URenameWorldWidget::BroadcastRenameButtonClicked(const FString& NewWorldName)
{
	if (!OnRenameButtonClicked.IsBound())
	{
		return;
	}

	OnRenameButtonClicked.Broadcast(NewWorldName);
}

void URenameWorldWidget::BroadcastCancelButtonClicked()
{
	if (!OnCancelButtonClicked.IsBound())
	{
		return;
	}

	OnCancelButtonClicked.Broadcast();
}

void URenameWorldWidget::WorldNameOnTextChanged(const FText& Text)
{
	FString TextString = Text.ToString();

	if (TextString.Len() > 32)
	{
		TextString = TextString.LeftChop(1);
	}

	WorldNameInputBox->SetText(FText::FromString(TextString));

	if (TextString.IsEmpty())
	{
		RenameButton->SetIsEnabled(false);
	}

	if (WorldOfSameNameAlreadyExists(TextString))
	{
		RenameButton->SetIsEnabled(false);
		ShowInvalidWarning(TEXT("World of same name already exists!"));
	}
	else if (WorldContainsInvalidCharacter(TextString))
	{
		RenameButton->SetIsEnabled(false);
		ShowInvalidWarning(TEXT("World name contains an invalid character!"));
	}
	else
	{
		RenameButton->SetIsEnabled(true);
		HideInvalidWarning();
	}
}
