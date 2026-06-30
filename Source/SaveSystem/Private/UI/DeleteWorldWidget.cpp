// Copyright Telephone Studios. All Rights Reserved.


#include "UI/DeleteWorldWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Interfaces/GameSaveLoadController.h"

UDeleteWorldWidget::UDeleteWorldWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	TitleTextBlock = nullptr;
	WarningTextBlock = nullptr;
	DeleteButton = nullptr;
	CancelButton = nullptr;

	WorldName = TEXT("");
}

void UDeleteWorldWidget::NativeConstruct()
{
	Super::NativeConstruct();

	DeleteButton->OnClicked.AddDynamic(this, &UDeleteWorldWidget::DeleteWorld);
	CancelButton->OnClicked.AddDynamic(this, &UDeleteWorldWidget::BroadcastCancelButtonClicked);
}

void UDeleteWorldWidget::Open(const FString& InWorldName)
{
	WorldName = InWorldName;

	const FString Title = FString::Printf(TEXT("Delete %s?"), *WorldName);
	const FString Warning = FString::Printf(TEXT("Are you sure you want to delete %s?"), *WorldName);

	TitleTextBlock->SetText(FText::FromString(Title));
	WarningTextBlock->SetText(FText::FromString(Warning));
}

void UDeleteWorldWidget::DeleteWorld()
{
	IGameSaveLoadController* GameSaveLoadController = Cast<IGameSaveLoadController>(GetGameInstance());
	if (GameSaveLoadController == nullptr)
	{
		return;
	}

	GameSaveLoadController->DeleteWorld(WorldName);

	BroadcastDeleteButtonClicked();
}

void UDeleteWorldWidget::BroadcastDeleteButtonClicked()
{
	if (!OnDeleteButtonClicked.IsBound())
	{
		return;
	}

	OnDeleteButtonClicked.Broadcast();
}

void UDeleteWorldWidget::BroadcastCancelButtonClicked()
{
	if (!OnCancelButtonClicked.IsBound())
	{
		return;
	}

	OnCancelButtonClicked.Broadcast();
}
