// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MultiOptionBox.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMultiOptionBoxValueChangedSignature, const FString&, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMultiOptionBoxValueChangedNoParamsSignature);

UCLASS()
class CUSTOMUI_API UMultiOptionBox : public UUserWidget
{
	GENERATED_BODY()
public:
	UMultiOptionBox(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	
	void AddOption(const FString& NewOption);
	void GiveQualityOptions();
	void SetSelectedOption(const FString& NewOption);
	void SetSelectedIndex(const int32& NewIndex);
	void ClearOptions();

	FOnMultiOptionBoxValueChangedSignature OnValueChanged;
	FOnMultiOptionBoxValueChangedNoParamsSignature OnValueChangedNoParams;

	FString GetSelectedOption() const;
	int32 GetSelectedIndex() const;

private:
	UPROPERTY(Meta = (BindWidget))
	class UButton* NextButton;
	UPROPERTY(Meta = (BindWidget))
	class UButton* PreviousButton;
	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* SelectedTextBlock;
	
	TArray<FString> Options;
	int32 SelectedOptionIndex;

	UFUNCTION()
	void NextOption();
	
	UFUNCTION()
	void PreviousOption();

	void Refresh();

};
