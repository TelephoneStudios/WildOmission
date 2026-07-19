// Copyright Telephone Studios. All Rights Reserved.


#include "ColorWheel.h"
#include "UI/Synth2DSlider.h"
#include "Kismet/KismetMathLibrary.h"

UColorWheel::UColorWheel(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	Wheel = nullptr;
	Lightness = 1.0f;
}

void UColorWheel::SetLightness(float InLightness)
{
	Lightness = InLightness;
}

void UColorWheel::NativeConstruct()
{
	Super::NativeConstruct();
	Wheel->OnValueChangedX.AddDynamic(this, &UColorWheel::OnColorWheelValueChanged);
}

void UColorWheel::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Clamp the slider within the wheel
	ClampSliderHandle();

	// Update wheel tint
	FSynth2DSliderStyle CurrentStyle = Wheel->WidgetStyle;
	CurrentStyle.BackgroundImage.TintColor = FSlateColor(FLinearColor(Lightness, Lightness, Lightness, 1.0f));
	Wheel->WidgetStyle = CurrentStyle;

	// Update handle color
	FLinearColor NewHandleColor = GetCurrentColor();
	Wheel->SetSliderHandleColor(NewHandleColor);

	Wheel->SynchronizeProperties();
}

void UColorWheel::SetColor(const FLinearColor& InColor)
{
	const FLinearColor HSVColor = InColor.LinearRGBToHSV();
	
	const float X = UKismetMathLibrary::DegCos(HSVColor.R) * HSVColor.G + 0.5f;
	const float Y = UKismetMathLibrary::DegSin(HSVColor.R) * HSVColor.G + 0.5f;

	Wheel->SetValue(FVector2D(X, Y));
}

FLinearColor UColorWheel::GetCurrentColor() const
{
	const FVector2D WheelValue = Wheel->GetValue();
	
	FLinearColor ReturnColor;
	
	float H = UKismetMathLibrary::ClampAxis(UKismetMathLibrary::DegAtan2(WheelValue.Y - 0.5f, WheelValue.X - 0.5f));
	float S = FMath::Clamp((FVector2D::Distance(FVector2D(0.5f, 0.5f), WheelValue) * 2.0f), 0.0f, 1.0f);
	UE_LOG(LogTemp, Warning, TEXT("Color Wheel Current Color H: %f"), ReturnColor.R);
	ReturnColor = FLinearColor::MakeFromHSV8(H, S * 255.0f, Lightness * 255.0f);

	//FLinearColor::HSV
	return ReturnColor;
}

void UColorWheel::ClampSliderHandle()
{
	const float MaxX = (UKismetMathLibrary::DegCos(GetAngle()) * 0.5f) + 0.5f;
	const float MaxY = (UKismetMathLibrary::DegSin(GetAngle()) * 0.5f) + 0.5f;

	if (!IsInRadius())
	{
		Wheel->SetValue(FVector2D(MaxX, MaxY));
	}
	else
	{
		Wheel->SetValue(Wheel->GetValue());
	}
}

bool UColorWheel::IsInRadius() const
{
	return UKismetMathLibrary::Distance2D(FVector2D(0.5f, 0.5f), Wheel->GetValue()) < 0.5f;
}

float UColorWheel::GetAngle() const
{
	return UKismetMathLibrary::ClampAxis(UKismetMathLibrary::DegAtan2(Wheel->GetValue().Y - 0.5f, Wheel->GetValue().X - 0.5f));
}

void UColorWheel::OnColorWheelValueChanged(float Value)
{
	if (OnColorChanged.IsBound())
	{
		OnColorChanged.Broadcast(GetCurrentColor());
	}
}
