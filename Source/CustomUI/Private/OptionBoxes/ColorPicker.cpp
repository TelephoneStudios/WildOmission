// Copyright Telephone Studios. All Rights Reserved.


#include "OptionBoxes/ColorPicker.h"
#include "OptionBoxes/SliderOptionBox.h"

UColorPicker::UColorPicker(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	
}

void UColorPicker::NativeConstruct()
{
	Super::NativeConstruct();

	HueSlider->SetMaxValue(255.0f);
	HueSlider->SetMinValue(0.0f);
	HueSlider->SetValue(0.0f);
	HueSlider->SetRoundAfterValueChanged(false);

	SaturationSlider->SetMaxValue(255.0f);
	SaturationSlider->SetMinValue(0.0f);
	SaturationSlider->SetValue(255.0f);
	SaturationSlider->SetRoundAfterValueChanged(false);

	LightnessSlider->SetMaxValue(255.0f);
	LightnessSlider->SetMinValue(0.0f);
	LightnessSlider->SetValue(255.0f);
	LightnessSlider->SetRoundAfterValueChanged(false);

	SetSelectedColor(FLinearColor::Red);
}

void UColorPicker::SetSelectedColor(const FLinearColor& InColor)
{
	uint8 H = 0, S = 0, V = 0;
	InColor.FGetHSV(H, S, V);
	HueSlider->SetValue(H);
	SaturationSlider->SetValue(S);
	LightnessSlider->SetValue(V);
}