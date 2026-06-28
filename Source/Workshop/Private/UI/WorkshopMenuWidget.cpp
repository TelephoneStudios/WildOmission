// Copyright Telephone Studios. All Rights Reserved.


#include "UI/WorkshopMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SteamHelperFunctionLibrary.h"
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
	USteamHelperFunctionLibrary::UploadWorkshopItem();
}

void UWorkshopMenuWidget::BackButtonClicked()
{
	if (OnBackButtonClicked.IsBound())
	{
		OnBackButtonClicked.Broadcast();
	}
}
