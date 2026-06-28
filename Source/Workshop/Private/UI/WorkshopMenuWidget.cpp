// Copyright Telephone Studios. All Rights Reserved.


#include "UI/WorkshopMenuWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/KismetSystemLibrary.h"
#include "WorkshopManager.h"
#include "Log.h"

UWorkshopMenuWidget::UWorkshopMenuWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UWorkshopMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UploadButton->OnClicked.AddDynamic(this, &UWorkshopMenuWidget::OnUploadButtonClicked);
	BackButton->OnClicked.AddDynamic(this, &UWorkshopMenuWidget::BackButtonClicked);
}

void UWorkshopMenuWidget::OnUploadButtonClicked()
{

	UWorkshopManager* WorkshopManager = UWorkshopManager::GetWorkshopManager();
	if (WorkshopManager == nullptr)
	{
		UE_LOG(LogWorkshop, Warning, TEXT("Failed to start upload, WorkshopManager returned nullptr"));
		return;
	}
	
	WorkshopManager->UploadWorkshopItem();

	FTimerDelegate CheckUploadStatusTimerDelegate;
	CheckUploadStatusTimerDelegate.BindUObject(this, &UWorkshopMenuWidget::CheckUploadStatus);
	GetWorld()->GetTimerManager().SetTimer(CheckUploadStatusTimerHandle, CheckUploadStatusTimerDelegate, 1.0f, true);
	MenuSwitcher->SetActiveWidget(UploadingMenu);

}

void UWorkshopMenuWidget::BackButtonClicked()
{
	if (OnBackButtonClicked.IsBound())
	{
		OnBackButtonClicked.Broadcast();
	}
}

void UWorkshopMenuWidget::CheckUploadStatus()
{
	UE_LOG(LogWorkshop, Display, TEXT("Checking workshop upload status"));
	bool Completed = UWorkshopManager::IsItemUpdateComplete(ItemUploadHandle);
	UE_LOG(LogWorkshop, Display, TEXT("Item upload completion status %i"), Completed);
	if (Completed)
	{
		MenuSwitcher->SetActiveWidget(BrowseMenu);
		GetWorld()->GetTimerManager().ClearTimer(CheckUploadStatusTimerHandle);
	}
}
