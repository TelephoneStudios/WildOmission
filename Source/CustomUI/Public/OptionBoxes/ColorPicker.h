// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ColorPicker.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnColorPickerColorSelectionChangedSignature, const FLinearColor&, NewColor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnColorPickerColorSelectionChangedNoParamsSignature);

UCLASS()
class CUSTOMUI_API UColorPicker : public UUserWidget
{
	GENERATED_BODY()

public:
	UColorPicker(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintAssignable)
	FOnColorPickerColorSelectionChangedSignature OnColorChanged;

	UPROPERTY(BlueprintAssignable)
	FOnColorPickerColorSelectionChangedNoParamsSignature OnColorChangedNoParams;

protected:

private:

};
