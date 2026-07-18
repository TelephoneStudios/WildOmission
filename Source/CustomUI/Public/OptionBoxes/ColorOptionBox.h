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

	virtual void NativeConstruct() override;

	FOnColorOptionBoxValueChangedSignature OnColorChanged;
	FOnColorOptionBoxValueChangedNoParamsSignature OnColorChangedNoParams;

	UFUNCTION(BlueprintCallable)
	void SetColor(const FLinearColor& NewColor, bool bUpdateUI = false);
	FLinearColor GetSelectedColor() const;


protected:
	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	class UButton* ColorPreview;
	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	class UTextBlock* ExpandTextBlock;
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	class UColorPicker* ColorPickerMenu;

	bool bColorPickerMenuOpen;
	FLinearColor CurrentColor;

	UFUNCTION(BlueprintCallable)
	void BroadcastColorChange();

private:
	UFUNCTION()
	void OnColorPreviewClicked();


};
