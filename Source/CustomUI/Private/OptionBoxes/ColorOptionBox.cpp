// Copyright Telephone Studios. All Rights Reserved.


#include "OptionBoxes/ColorOptionBox.h"
#include "OptionBoxes/SliderOptionBox.h"
#include "Color/ColorWheel.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

UColorOptionBox::UColorOptionBox(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	ColorPreview = nullptr;
	ExpandTextBlock = nullptr;
	bColorMenuOpen = false;
	ColorMenu = nullptr;
	ColorWheel = nullptr;
	LightnessSlider = nullptr;

}

void UColorOptionBox::NativeConstruct()
{
	Super::NativeConstruct();

	LightnessSlider->SetMaxValue(255.0f);
	LightnessSlider->SetMinValue(0.0f);
	LightnessSlider->SetValue(255.0f);
	LightnessSlider->SetRoundAfterValueChanged(false);
	LightnessSlider->OnValueChanged.AddDynamic(this, &UColorOptionBox::OnLightnessSliderValueChanged);
	ColorWheel->OnColorChanged.AddDynamic(this, &UColorOptionBox::OnColorWheelValueChanged);
	
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
	FLinearColor FixedColor = FLinearColor(NewColor.R, NewColor.G, NewColor.B, 1.0f);
	ColorPreview->SetBackgroundColor(FixedColor);
	ColorWheel->SetColor(NewColor);
	LightnessSlider->SetValue(NewColor.LinearRGBToHSV().B * 255.0f);
	ColorWheel->SetLightness(NewColor.LinearRGBToHSV().B);

	if (bUpdateUI)
	{
		// TODO update color wheel
	}
}

FLinearColor UColorOptionBox::GetSelectedColor() const
{
	return ColorWheel->GetCurrentColor();
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

void UColorOptionBox::OnColorWheelValueChanged(const FLinearColor& NewColor)
{
	const FLinearColor FixedColor = FLinearColor(NewColor.R, NewColor.G, NewColor.B, 1.0f);
	ColorPreview->SetBackgroundColor(FixedColor);

	BroadcastColorChange();
}

void UColorOptionBox::OnLightnessSliderValueChanged(float Value)
{
	ColorWheel->SetLightness(Value / 255.0f);
	ColorPreview->SetBackgroundColor(ColorWheel->GetCurrentColor());

	BroadcastColorChange();
}

void UColorOptionBox::BroadcastColorChange()
{
	if (OnColorChanged.IsBound())
	{
		OnColorChanged.Broadcast(GetSelectedColor());
	}

	if (OnColorChangedNoParams.IsBound())
	{
		OnColorChangedNoParams.Broadcast();
	}
}