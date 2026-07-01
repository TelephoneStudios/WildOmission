// Copyright Telephone Studios. All Rights Reserved.


#include "UI/WorkshopMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ProgressBar.h"
#include "UI/WorldSelectionWidget.h"
#include "UI/WorkshopUploadMenuWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "WorkshopManager.h"
#include "SaveManager.h"
#include "Log.h"

UWorkshopMenuWidget::UWorkshopMenuWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UWorkshopMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UploadButton->OnClicked.AddDynamic(this, &UWorkshopMenuWidget::OpenWorldSelectionForUploading);
	BackButton->OnClicked.AddDynamic(this, &UWorkshopMenuWidget::BackButtonClicked);

	WorldSelectionMenu->OnSelectButtonClicked.AddDynamic(this, &UWorkshopMenuWidget::OpenUploadWorldMenu);
	WorldSelectionMenu->OnCancelButtonClicked.AddDynamic(this, &UWorkshopMenuWidget::OpenWorkshopMenu);

	UploadWorldMenu->OnCancelButtonClicked.AddDynamic(this, &UWorkshopMenuWidget::OpenWorldSelectionForUploading);
	UploadWorldMenu->OnUploadButtonClicked.AddDynamic(this, &UWorkshopMenuWidget::UploadWorld);

	UWorkshopManager* WorkshopManager = UWorkshopManager::GetWorkshopManager();
	if (WorkshopManager)
	{
		WorkshopManager->OnWorkshopItemSubmitted.AddDynamic(this, &UWorkshopMenuWidget::OnUploadSubmitted);
	}
}

void UWorkshopMenuWidget::NativeTick(const FGeometry& MyGeomotry, float InDeltaTime)
{
	Super::NativeTick(MyGeomotry, InDeltaTime);

	UWorkshopManager* WorkshopManager = UWorkshopManager::GetWorkshopManager();
	if (WorkshopManager == nullptr || UploadProgressBar == nullptr)
	{
		return;
	}

	if (MenuSwitcher->GetActiveWidget() == UploadingMenu)
	{
		float Percent = 0.0f;
		EItemUpdateStatus Status = WorkshopManager->GetItemUploadStatus(Percent);
		if (!WorkshopManager->IsUploadInProgress())
		{
			OpenWorkshopMenu();
		}

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
		UploadProgressTextBlock->SetText(FText::FromString(StatusString));
		UploadProgressBar->SetPercent(Percent);
	}
}

void UWorkshopMenuWidget::BackButtonClicked()
{
	if (OnBackButtonClicked.IsBound())
	{
		OnBackButtonClicked.Broadcast();
	}
}

void UWorkshopMenuWidget::OnUploadSubmitted()
{
	UE_LOG(LogWorkshop, Display, TEXT("Item updates submitted"));	
	MenuSwitcher->SetActiveWidget(WorkshopMenu);
}

void UWorkshopMenuWidget::OpenWorkshopMenu()
{
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

	MenuSwitcher->SetActiveWidget(UploadingMenu);
}
