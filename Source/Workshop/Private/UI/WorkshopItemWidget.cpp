// Copyright Telephone Studios. All Rights Reserved.


#include "UI/WorkshopItemWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Log.h"

UWorkshopItemWidget::UWorkshopItemWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	SetIsFocusable(true);

	Button = nullptr;
	NameTextBlock = nullptr;
	PreviewImage = nullptr;
	
}
void UWorkshopItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Button->OnClicked.AddDynamic(this, &UWorkshopItemWidget::OnButtonClicked);
}

void UWorkshopItemWidget::Setup(const FSteamWorkshopItemDetails& Details)
{
	NameTextBlock->SetText(FText::FromString(Details.Title));
	FileID = Details.FileID;
	PreviewImage->SetBrushFromTexture(Details.PreviewTexture);

	// todo set preview image
	// todo set identification
}

void UWorkshopItemWidget::NativeTick(const FGeometry& MyGeomotry, float InDeltaTime)
{
	Super::NativeTick(MyGeomotry, InDeltaTime);

}

void UWorkshopItemWidget::OnButtonClicked()
{
	if (OnClicked.IsBound())
	{
		OnClicked.Broadcast(FileID);
	}
}