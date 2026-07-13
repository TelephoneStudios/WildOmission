// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SliderOptionBox.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSliderOptionBoxValueChangedSignature, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSliderOptionBoxValueChangedNoParamsSignature);

UCLASS()
class CUSTOMUI_API USliderOptionBox : public UUserWidget
{
	GENERATED_BODY()

public:
	USliderOptionBox(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	FOnSliderOptionBoxValueChangedSignature OnValueChanged;
	FOnSliderOptionBoxValueChangedNoParamsSignature OnValueChangedNoParams;

	UFUNCTION(BlueprintCallable)
	void SetValue(float Value);
	UFUNCTION(BlueprintCallable)
	void SetMinValue(float Value);
	UFUNCTION(BlueprintCallable)
	void SetMaxValue(float Value);
	UFUNCTION(BlueprintCallable)
	float GetValue() const;

	UFUNCTION(BlueprintCallable)
	void SetShowDecimal(bool Show);
	UFUNCTION(BlueprintCallable)
	bool GetShowDecimal() const;
	
	UFUNCTION(BlueprintCallable)
	void SetRoundAfterValueChanged(bool Round);
	UFUNCTION(BlueprintCallable)
	bool GetRoundAfterValueChanged() const;

protected:
	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	class USlider* Slider;
	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	class UTextBlock* TextBlock;
	bool ShowDecimal;
	bool RoundAfterValueChanged;
private:

	UFUNCTION()
	void OnSliderValueChanged(float Value);

};
