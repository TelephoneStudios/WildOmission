// Copyright Telephone Studios. All Rights Reserved.


#include "OptionBoxes/ColorOptionBox.h"
#include "OptionBoxes/ColorPicker.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

UColorOptionBox::UColorOptionBox(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	ColorPreview = nullptr;
	ExpandTextBlock = nullptr;
	bColorMenuOpen = false;
	ColorMenu = nullptr;

	LightnessSlider = nullptr;

}

void UColorOptionBox::NativeConstruct()
{
	Super::NativeConstruct();
	ColorPreview->OnClicked.AddDynamic(this, &UColorOptionBox::OnColorPreviewClicked);
	ExpandTextBlock->SetText(FText::FromString(TEXT("<")));
	ColorMenu->SetVisibility(ESlateVisibility::Collapsed);
}

void UColorOptionBox::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UColorOptionBox::SetColor(const FLinearColor& NewColor, bool bUpdateUI)
{
	// do this so the alpha isn't transparent
	ColorPreview->SetBackgroundColor(CurrentColor);
	FLinearColor FixedColor = FLinearColor(NewColor.R, NewColor.G, NewColor.B, 1.0f);
	CurrentColor = FixedColor;

	if (bUpdateUI)
	{
		//ColorMenu->SetSelectedColor(NewColor);
	}
}

FLinearColor UColorOptionBox::GetSelectedColor() const
{
	return CurrentColor;
}

void UColorOptionBox::OnColorPreviewClicked()
{
	if (bColorMenuOpen == false)
	{
		ExpandTextBlock->SetText(FText::FromString(TEXT("v")));
		ColorMenu->SetVisibility(ESlateVisibility::Visible);
		bColorMenuOpen = true;
	}
	else
	{
		ExpandTextBlock->SetText(FText::FromString(TEXT("<")));
		ColorMenu->SetVisibility(ESlateVisibility::Collapsed);
		bColorMenuOpen = false;
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