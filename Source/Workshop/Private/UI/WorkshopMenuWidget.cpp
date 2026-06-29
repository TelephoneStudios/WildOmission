// Copyright Telephone Studios. All Rights Reserved.


#include "UI/WorkshopMenuWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ProgressBar.h"
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

	UploadProgressBar->SetPercent(WorkshopManager->GetItemUploadPercentage());
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
	//CheckUploadStatusTimerDelegate.BindUObject(this, &UWorkshopMenuWidget::CheckUploadStatus);
	//GetWorld()->GetTimerManager().SetTimer(CheckUploadStatusTimerHandle, CheckUploadStatusTimerDelegate, 1.0f, true);
	MenuSwitcher->SetActiveWidget(UploadingMenu);

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
