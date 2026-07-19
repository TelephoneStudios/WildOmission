// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ColorWheel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnColorWheelValueChangedSignature, const FLinearColor&, NewColor);

UCLASS()
class CUSTOMUI_API UColorWheel : public UUserWidget
{
	GENERATED_BODY()

public:
	UColorWheel(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable)
	FOnColorWheelValueChangedSignature OnColorChanged;

	UFUNCTION(BlueprintCallable)
	FLinearColor GetCurrentColor() const;
	
	UFUNCTION(BlueprintCallable)
	void SetColor(const FLinearColor& InColor);
	
	UFUNCTION(BlueprintCallable)
	void SetLightness(float InLightness);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	class USynth2DSlider* Wheel;

	UPROPERTY()
	float Lightness;

private:
	void ClampSliderHandle();

	bool IsInRadius() const;
	float GetAngle() const;

	UFUNCTION()
	void OnColorWheelValueChanged(float Value);
};
