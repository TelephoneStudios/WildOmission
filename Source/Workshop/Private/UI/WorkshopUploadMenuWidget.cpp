// Copyright Telephone Studios. All Rights Reserved.


#include "UI/WorkshopUploadMenuWidget.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Image.h"
#include "Kismet/KismetSystemLibrary.h"
#include "WorkshopManager.h"
#include "SaveManager.h"
#include "WorldInformation.h"
#include "WorldData.h"
#include "Engine/Texture2D.h"
#include "Log.h"

UWorkshopUploadMenuWidget::UWorkshopUploadMenuWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UWorkshopUploadMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UploadButton->OnClicked.AddDynamic(this, &UWorkshopUploadMenuWidget::UploadButtonClicked);
	CancelButton->OnClicked.AddDynamic(this, &UWorkshopUploadMenuWidget::CancelButtonClicked);
}

void UWorkshopUploadMenuWidget::SetWorld(const FString& InWorldName)
{
	WorldName = InWorldName;

	WorldNameTextBlock->SetText(FText::FromString(WorldName));

	NameInputBox->SetText(FText::FromString(WorldName));

	UTexture2D* WorldIconTexture = ASaveManager::GetWorldIcon(WorldName);

	WorldIcon->SetBrushFromTexture(WorldIconTexture);
}

void UWorkshopUploadMenuWidget::NativeTick(const FGeometry& MyGeomotry, float InDeltaTime)
{
	Super::NativeTick(MyGeomotry, InDeltaTime);
}

void UWorkshopUploadMenuWidget::UploadButtonClicked()
{
	if (OnUploadButtonClicked.IsBound()
		&& NameInputBox && DescriptionInputBox)
	{
		OnUploadButtonClicked.Broadcast(WorldName, NameInputBox->GetText().ToString(), DescriptionInputBox->GetText().ToString());
	}
}

void UWorkshopUploadMenuWidget::CancelButtonClicked()
{
	if (OnCancelButtonClicked.IsBound())
	{
		OnCancelButtonClicked.Broadcast();
	}
}