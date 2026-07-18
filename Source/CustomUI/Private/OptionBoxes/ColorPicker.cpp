// Copyright Telephone Studios. All Rights Reserved.


#include "OptionBoxes/ColorPicker.h"
#include "OptionBoxes/SliderOptionBox.h"

UColorPicker::UColorPicker(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	LightnessSlider = nullptr;
}

void UColorPicker::NativeConstruct()
{
	Super::NativeConstruct();

	LightnessSlider->SetMaxValue(255.0f);
	LightnessSlider->SetMinValue(0.0f);
	LightnessSlider->SetValue(255.0f);
	LightnessSlider->SetRoundAfterValueChanged(false);

	SetSelectedColor(FLinearColor::Red);
}

void UColorPicker::SetSelectedColor(const FLinearColor& InColor)
{
	LightnessSlider->SetValue(InColor.LinearRGBToHSV().B * 255.0f);
	UpdateColorWheel(InColor);
}