// Copyright Telephone Studios. All Rights Reserved.


#include "ItemIconWidget.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "UI/CreativeMenuWidget.h"
#include "Components/InventoryComponent.h"
#include "Structs/ItemData.h"
#include "Color/UIColors.h"

void UItemIconWidget::Setup(UCreativeMenuWidget* InParentMenu, const FName& InItemID)
{
	ParentMenu = InParentMenu;
	
	ItemID = InItemID;

	FItemData* ItemData = UInventoryComponent::GetItemData(ItemID);
	if (ItemData)
	{
		ItemIconBorder->SetBrushFromMaterial(ItemData->Thumbnail);
	}

	ItemButton->OnClicked.AddDynamic(this, &UItemIconWidget::OnClicked);
}

bool UItemIconWidget::IsSelected() const
{
	if (ParentMenu == nullptr)
	{
		return false;
	}

	return ParentMenu->GetSelectedItem() == this->ItemID;
}

void UItemIconWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	if (IsSelected())
	{
		FUIColor* Blue = UUIColors::GetBaseColor(FName("Blue"));
		if (Hovering)
		{
			BackgroundBorder->SetBrushColor(UUIColors::GetHighlightedColor(Blue) - FLinearColor(0.0f, 0.0f, 0.0f, 0.7f));
		}
		else
		{
			BackgroundBorder->SetBrushColor(Blue->Default - FLinearColor(0.0f, 0.0f, 0.0f, 0.7f));
		}
	}
	else
	{
		FUIColor* White = UUIColors::GetBaseColor(FName("White"));

		if (Hovering)
		{
			BackgroundBorder->SetBrushColor(UUIColors::GetHighlightedColor(White) - FLinearColor(0.0f, 0.0f, 0.0f, 0.7f));
		}
		else
		{
			BackgroundBorder->SetBrushColor(White->Default - FLinearColor(0.0f,0.0f,0.0f,0.7f));
		}
	}
}

void UItemIconWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	Hovering = true;
}

void UItemIconWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	Hovering = false;
}

void UItemIconWidget::OnClicked()
{
	ParentMenu->SetSelectedItem(this->ItemID);
}