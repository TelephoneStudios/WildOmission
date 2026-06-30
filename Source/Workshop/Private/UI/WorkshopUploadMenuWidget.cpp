// Copyright Telephone Studios. All Rights Reserved.


#include "UI/WorkshopUploadMenuWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ProgressBar.h"
#include "Kismet/KismetSystemLibrary.h"
#include "WorkshopManager.h"
#include "Log.h"

UWorkshopUploadMenuWidget::UWorkshopUploadMenuWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UWorkshopUploadMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CancelButton->OnClicked.AddDynamic(this, &UWorkshopUploadMenuWidget::CancelButtonClicked);
}

void UWorkshopUploadMenuWidget::SetWorld(const FString& InWorldName)
{

}

void UWorkshopUploadMenuWidget::NativeTick(const FGeometry& MyGeomotry, float InDeltaTime)
{
	Super::NativeTick(MyGeomotry, InDeltaTime);
}

void UWorkshopUploadMenuWidget::UploadButtonClicked()
{
}

void UWorkshopUploadMenuWidget::CancelButtonClicked()
{
	if (OnCancelButtonClicked.IsBound())
	{
		OnCancelButtonClicked.Broadcast();
	}
}