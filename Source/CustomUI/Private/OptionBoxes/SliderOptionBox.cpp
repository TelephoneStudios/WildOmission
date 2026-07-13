// Copyright Telephone Studios. All Rights Reserved.


#include "OptionBoxes/SliderOptionBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"

USliderOptionBox::USliderOptionBox(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	Slider = nullptr;
	TextBlock = nullptr;

	ShowDecimal = false;
	RoundAfterValueChanged = true;
}

void USliderOptionBox::NativeConstruct()
{
	Super::NativeConstruct();

	Slider->OnValueChanged.AddDynamic(this, &USliderOptionBox::OnSliderValueChanged);
}

void USliderOptionBox::SetValue(float Value)
{
	Slider->SetValue(Value);
	OnSliderValueChanged(Value);
}

void USliderOptionBox::SetMinValue(float Value)
{
	Slider->SetMinValue(Value);
	OnSliderValueChanged(Value);
}

void USliderOptionBox::SetMaxValue(float Value)
{
	Slider->SetMaxValue(Value);
	OnSliderValueChanged(Value);
}

float USliderOptionBox::GetValue() const
{
	return Slider->GetValue();
}

void USliderOptionBox::SetShowDecimal(bool Show)
{
	ShowDecimal = Show;
}

bool USliderOptionBox::GetShowDecimal() const
{
	return ShowDecimal;
}

void USliderOptionBox::SetRoundAfterValueChanged(bool Round)
{
	RoundAfterValueChanged = Round;
}

bool USliderOptionBox::GetRoundAfterValueChanged() const
{
	return RoundAfterValueChanged;
}

void USliderOptionBox::OnSliderValueChanged(float Value)
{
	if (RoundAfterValueChanged)
	{
		Value = FMath::RoundToFloat(Value);
	}

	FString ValueString;
	if (ShowDecimal)
	{
		ValueString = FString::Printf(TEXT("%f"), Value);
	}
	else
	{
		ValueString = FString::Printf(TEXT("%i"), FMath::RoundToInt32(Value));
	}

	TextBlock->SetText(FText::FromString(ValueString));
	Slider->SetValue(Value);

	if (OnValueChanged.IsBound())
	{
		OnValueChanged.Broadcast(Value);
	}

	if (OnValueChangedNoParams.IsBound())
	{
		OnValueChangedNoParams.Broadcast();
	}
}
