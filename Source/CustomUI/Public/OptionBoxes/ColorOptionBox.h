// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ColorOptionBox.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnColorOptionBoxValueChangedSignature, const FLinearColor&, NewColor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnColorOptionBoxValueChangedNoParamsSignature);

UCLASS()
class CUSTOMUI_API UColorOptionBox : public UUserWidget
{
	GENERATED_BODY()

public:
	UColorOptionBox(const FObjectInitializer& ObjectInitializer);


	UPROPERTY(BlueprintAssignable)
	FOnColorOptionBoxValueChangedSignature OnColorChanged;
	UPROPERTY(BlueprintAssignable)
	FOnColorOptionBoxValueChangedNoParamsSignature OnColorChangedNoParams;

	UFUNCTION(BlueprintCallable)
	void SetColor(const FLinearColor& NewColor, bool bUpdateUI = false);
	FLinearColor GetSelectedColor() const;


protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	class UButton* ColorPreview;
	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	class UTextBlock* ExpandTextBlock;
	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	class UWidget* ColorMenu;

	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	class UColorWheel* ColorWheel;
	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	class USliderOptionBox* LightnessSlider;


	bool bColorMenuOpen;
	FLinearColor CurrentColor;

	UFUNCTION(BlueprintCallable)
	void BroadcastColorChange();

private:
	UFUNCTION()
	void OnColorPreviewClicked();
	UFUNCTION()
	void OnColorWheelValueChanged(const FLinearColor& NewColor);
	UFUNCTION()
	void OnLightnessSliderValueChanged(float Value);


};
