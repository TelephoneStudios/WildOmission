// Copyright Telephone Studios. All Rights Reserved.


#include "OptionBoxes/ColorOptionBox.h"
#include "OptionBoxes/ColorPicker.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

UColorOptionBox::UColorOptionBox(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	ColorPreview = nullptr;
	ExpandTextBlock = nullptr;
	ColorPickerMenu = nullptr;
	bColorPickerMenuOpen = false;
}

void UColorOptionBox::NativeConstruct()
{
	Super::NativeConstruct();
	ColorPreview->OnClicked.AddDynamic(this, &UColorOptionBox::OnColorPreviewClicked);
	ExpandTextBlock->SetText(FText::FromString(TEXT("<")));
	ColorPickerMenu->SetVisibility(ESlateVisibility::Collapsed);
}

void UColorOptionBox::SetColor(const FLinearColor& NewColor, bool bUpdateUI)
{
	// do this so the alpha isn't transparent
	FLinearColor FixedColor = FLinearColor(NewColor.R, NewColor.G, NewColor.B, 1.0f);
	ColorPreview->SetBackgroundColor(FixedColor);
	CurrentColor = FixedColor;

	if (bUpdateUI)
	{
		ColorPickerMenu->SetSelectedColor(NewColor);
	}
}

FLinearColor UColorOptionBox::GetSelectedColor() const
{
	return CurrentColor;
}

void UColorOptionBox::OnColorPreviewClicked()
{
	if (bColorPickerMenuOpen == false)
	{
		ExpandTextBlock->SetText(FText::FromString(TEXT("v")));
		ColorPickerMenu->SetVisibility(ESlateVisibility::Visible);
		bColorPickerMenuOpen = true;
	}
	else
	{
		ExpandTextBlock->SetText(FText::FromString(TEXT("<")));
		ColorPickerMenu->SetVisibility(ESlateVisibility::Collapsed);
		bColorPickerMenuOpen = false;
	}
}

void UColorOptionBox::BroadcastColorChange()
{
	if (OnColorChanged.IsBound())
	{
		OnColorChanged.Broadcast(CurrentColor);
	}

	if (OnColorChangedNoParams.IsBound())
	{
		OnColorChangedNoParams.Broadcast();
	}
}