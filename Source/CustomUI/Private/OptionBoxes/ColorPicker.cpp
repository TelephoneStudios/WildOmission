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
	uint8 H = 0, S = 0, V = 0;
	InColor.FGetHSV(H, S, V);
	LightnessSlider->SetValue(V);
	UpdateColorWheel(InColor);

}