// Copyright Telephone Studios. All Rights Reserved.


#include "UI/WorkshopMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/WrapBox.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ProgressBar.h"
#include "UI/WorkshopItemWidget.h"
#include "UI/WorldSelectionWidget.h"
#include "UI/WorkshopUploadMenuWidget.h"
#include "SteamHelperFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SaveManager.h"
#include "Log.h"

UWorkshopMenuWidget::UWorkshopMenuWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	SetIsFocusable(true);

	// Workshop menu
	MenuSwitcher = nullptr;
	WorkshopItemsWrapBox = nullptr;
	RefreshButton = nullptr;
	UploadButton = nullptr;
	BackButton = nullptr;
	WorkshopMenu = nullptr;
	WorkshopItemClass = nullptr;

	// Uploading
	LoadingMenu = nullptr;
	LoadingMenuTitleTextBlock = nullptr;
	LoadingProgressTextBlock = nullptr;
	LoadingProgressBar = nullptr;
	WorldSelectionMenu = nullptr;
	UploadWorldMenu = nullptr;

	// Details panel
	SelectedItemDetailsPanel = nullptr;
	SelectedItemTitleTextBlock = nullptr;
	SelectedItemAuthorTextBlock = nullptr;
	SelectedItemImage = nullptr;
	OpenWorkshopPageForSelectedButton = nullptr;
	DownloadSelectedWorldButton = nullptr;

	static ConstructorHelpers::FClassFinder<UWorkshopItemWidget> WorkshopItemWidgetBlueprint(TEXT("/Game/Workshop/UI/WBP_WorkshopItem"));
	if (WorkshopItemWidgetBlueprint.Succeeded())
	{
		WorkshopItemClass = WorkshopItemWidgetBlueprint.Class;
	}
}

void UWorkshopMenuWidget::OnOpen()
{
	Refresh();

}

void UWorkshopMenuWidget::Refresh()
{
	UWorkshopManager* WorkshopManager = UWorkshopManager::GetWorkshopManager();
	if (WorkshopManager == nullptr)
	{
		UE_LOG(LogWorkshop, Warning, TEXT("Failed to refresh workshop menu, couldn't get workshop manager"));
		return;
	}
	
	SelectedItem.Reset();
	SelectedItemDetailsPanel->SetVisibility(ESlateVisibility::Collapsed);
	MenuSwitcher->SetActiveWidget(WorkshopMenu);

	WorkshopItemsWrapBox->ClearChildren();
	WorkshopManager->QueryPopularWorlds();
}

void UWorkshopMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshButton->OnClicked.AddDynamic(this, &UWorkshopMenuWidget::Refresh);
	UploadButton->OnClicked.AddDynamic(this, &UWorkshopMenuWidget::OpenWorldSelectionForUploading);
	BackButton->OnClicked.AddDynamic(this, &UWorkshopMenuWidget::BackButtonClicked);

	WorldSelectionMenu->OnSelectButtonClicked.AddDynamic(this, &UWorkshopMenuWidget::OpenUploadWorldMenu);
	WorldSelectionMenu->OnCancelButtonClicked.AddDynamic(this, &UWorkshopMenuWidget::OpenWorkshopMenu);

	UploadWorldMenu->OnCancelButtonClicked.AddDynamic(this, &UWorkshopMenuWidget::OpenWorldSelectionForUploading);
	UploadWorldMenu->OnUploadButtonClicked.AddDynamic(this, &UWorkshopMenuWidget::UploadWorld);

	OpenWorkshopPageForSelectedButton->OnClicked.AddDynamic(this, &UWorkshopMenuWidget::OpenWorkshopForSelectedWorld);
	DownloadSelectedWorldButton->OnClicked.AddDynamic(this, &UWorkshopMenuWidget::DownloadWorld);
	SelectedItemDetailsPanel->SetVisibility(ESlateVisibility::Collapsed);

	UWorkshopManager* WorkshopManager = UWorkshopManager::GetWorkshopManager();
	if (WorkshopManager)
	{
		WorkshopManager->OnWorkshopQueryCompleted.AddDynamic(this, &UWorkshopMenuWidget::OnQueryCompleted);
		WorkshopManager->OnWorkshopItemSubmitted.AddDynamic(this, &UWorkshopMenuWidget::OnUploadSubmitted);
	}
}

void UWorkshopMenuWidget::NativeTick(const FGeometry& MyGeomotry, float InDeltaTime)
{
	Super::NativeTick(MyGeomotry, InDeltaTime);

	UWorkshopManager* WorkshopManager = UWorkshopManager::GetWorkshopManager();
	if (WorkshopManager == nullptr || LoadingProgressBar == nullptr)
	{
		return;
	}

	if (MenuSwitcher->GetActiveWidget() == LoadingMenu)
	{
		float Percent = 0.0f;
		if (WorkshopManager->IsUploadInProgress())
		{
			EItemUpdateStatus Status = WorkshopManager->GetItemUploadStatus(Percent);
			FString StatusString;
			switch (Status)
			{
			case EItemUpdateStatus::k_EItemUpdateStatusPreparingConfig:
				StatusString = TEXT("Preparing Config...");
				break;
			case EItemUpdateStatus::k_EItemUpdateStatusPreparingContent:
				StatusString = TEXT("Preparing Content...");
				break;
			case EItemUpdateStatus::k_EItemUpdateStatusUploadingContent:
				StatusString = TEXT("Uploading Content...");
				break;
			case EItemUpdateStatus::k_EItemUpdateStatusUploadingPreviewFile:
				StatusString = TEXT("Uploading Preview File...");
				break;
			case EItemUpdateStatus::k_EItemUpdateStatusCommittingChanges:
				StatusString = TEXT("Committing Changes...");
				break;

			}
			LoadingProgressTextBlock->SetText(FText::FromString(StatusString));
			LoadingProgressBar->SetPercent(Percent);
		}
		else if (WorkshopManager->IsDownloadInProgress())
		{
			Percent = WorkshopManager->GetItemDownloadProgress();
			LoadingProgressTextBlock->SetText(FText::FromString(TEXT("Downloading...")));
			LoadingProgressBar->SetPercent(Percent);
		}
		else
		{
			OpenWorkshopMenu();
		}
	}
}

void UWorkshopMenuWidget::SelectWorkshopItem(const FSteamWorkshopItemDetails& Details)
{
	UWorkshopManager* WorkshopManager = UWorkshopManager::GetWorkshopManager();
	if (WorkshopManager == nullptr)
	{
		return;
	}

	const bool IsSubscribed = WorkshopManager->IsWorkshopItemSubscribed(FCString::Atoi64(*Details.FileID));

	DownloadSelectedWorldButton->SetIsEnabled(!IsSubscribed);
	const FString DownloadString = IsSubscribed ? TEXT("Already downloaded") : TEXT("Download");
	DownloadButtonTextBlock->SetText(FText::FromString(DownloadString));
	
	SelectedItem = Details;

	// Show the details panel
	SelectedItemDetailsPanel->SetVisibility(ESlateVisibility::Visible);

	// Set title, author, and description
	SelectedItemTitleTextBlock->SetText(FText::FromString(Details.Title));

	const FString AuthorString = TEXT("by: ") + Details.AuthorName;
	SelectedItemAuthorTextBlock->SetText(FText::FromString(AuthorString));
	
	FString Description;
	if (!Details.Description.IsEmpty())
	{
		Description = Details.Description;
	}
	else
	{
		Description = TEXT("No description provided.");
	}
	SelectedItemDescriptionTextBlock->SetText(FText::FromString(Description));
	
	// Set preview
	if (Details.PreviewTexture)
	{
		SelectedItemImage->SetBrushFromTexture(Details.PreviewTexture);
	}
}

void UWorkshopMenuWidget::BackButtonClicked()
{
	if (OnBackButtonClicked.IsBound())
	{
		OnBackButtonClicked.Broadcast();
	}
}

void UWorkshopMenuWidget::OnQueryCompleted(bool bSuccess, const TArray<FSteamWorkshopItemDetails>& Items)
{
	for (const FSteamWorkshopItemDetails& ItemDetails : Items)
	{
		// Create item widget
		UWorkshopItemWidget* NewWorkshopItemWidget = CreateWidget<UWorkshopItemWidget>(this, WorkshopItemClass);
		if (NewWorkshopItemWidget == nullptr)
		{
			continue;
		}
		// Populate new widget with details
		NewWorkshopItemWidget->Setup(ItemDetails);
		NewWorkshopItemWidget->OnClicked.AddDynamic(this, &UWorkshopMenuWidget::SelectWorkshopItem);
		// Add to wrap box
		WorkshopItemsWrapBox->AddChild(NewWorkshopItemWidget);
		// Give it padding
		NewWorkshopItemWidget->SetPadding(FMargin(5.0f));
	}
}

void UWorkshopMenuWidget::OnUploadSubmitted()
{
	UE_LOG(LogWorkshop, Display, TEXT("Item updates submitted"));	
	OpenWorkshopMenu();
}

void UWorkshopMenuWidget::OpenWorkshopMenu()
{
	SelectedItem.Reset();
	SelectedItemDetailsPanel->SetVisibility(ESlateVisibility::Collapsed);
	MenuSwitcher->SetActiveWidget(WorkshopMenu);
}

void UWorkshopMenuWidget::OpenWorldSelectionForUploading()
{
	MenuSwitcher->SetActiveWidget(WorldSelectionMenu);

	TArray<FString> WorldFolderNames = ASaveManager::GetAllWorldFolderNames();
	WorldSelectionMenu->SetWorldList(WorldFolderNames);
}

void UWorkshopMenuWidget::OpenUploadWorldMenu()
{
	MenuSwitcher->SetActiveWidget(UploadWorldMenu);
	UploadWorldMenu->SetWorld(WorldSelectionMenu->SelectedWorldName.GetValue());
}

void UWorkshopMenuWidget::UploadWorld(const FString& WorldName, const FString& WorkshopItemName, const FString& WorkshopItemDescription)
{
	UWorkshopManager* WorkshopManager = UWorkshopManager::GetWorkshopManager();
	if(WorkshopManager == nullptr)
	{
		return;
	}
	UE_LOG(LogTemp, Display, TEXT("UWorkshopMenuWidget, uploading world: %s"), *WorldName);
	WorkshopManager->UploadWorld(WorldName, WorkshopItemName, WorkshopItemDescription);
	LoadingMenuTitleTextBlock->SetText(FText::FromString(TEXT("World is uploading... please wait.")));
	MenuSwitcher->SetActiveWidget(LoadingMenu);
}

void UWorkshopMenuWidget::OpenWorkshopForSelectedWorld()
{
	if (!SelectedItem.IsSet())
	{
		return;
	}

	USteamHelperFunctionLibrary::OpenWebPageInOverlay(SelectedItem.GetValue().WorkshopURL);
}

void UWorkshopMenuWidget::DownloadWorld()
{
	UWorkshopManager* WorkshopManager = UWorkshopManager::GetWorkshopManager();
	if (WorkshopManager == nullptr)
	{
		UE_LOG(LogWorkshop, Warning, TEXT("Failed to download world, couldn't get Workshop Manager"));
		return;
	}
	if (!SelectedItem.IsSet())
	{
		UE_LOG(LogWorkshop, Warning, TEXT("Failed to download world, selected isn't set"));
		return;
	}

	WorkshopManager->SubscribeAndDownloadWorld(SelectedItem.GetValue().FileID);
	LoadingMenuTitleTextBlock->SetText(FText::FromString(TEXT("Downloading World... please wait.")));
	UE_LOG(LogWorkshop, Display, TEXT("Bringing up loading menu."));
	MenuSwitcher->SetActiveWidget(LoadingMenu);
}