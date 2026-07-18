// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterCustomizationWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterCustomizationMenuBackButtonClickedSignature);

UCLASS()
class CHARACTERCUSTOMIZATION_API UCharacterCustomizationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UCharacterCustomizationWidget(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION()
	void OnOpen();

	FOnCharacterCustomizationMenuBackButtonClickedSignature OnBackButtonClicked;

protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(Meta = (BindWidget))
	class UMultiOptionBox* GenderMultiOptionBox;
	UPROPERTY(Meta = (BindWidget))
	class UColorOptionBox* SkinColorOptionBox;
	UPROPERTY(Meta = (BindWidget))
	class UColorOptionBox* ShirtColorOptionBox;
	UPROPERTY(Meta = (BindWidget))
	class UColorOptionBox* PantsColorOptionBox;
	UPROPERTY(Meta = (BindWidget))
	class UColorOptionBox* ShoeColorOptionBox;
	UPROPERTY(Meta = (BindWidget))
	class UButton* BackButton;

private:
	UFUNCTION()
	void OnSettingChange();

	UFUNCTION()
	void OnBackClicked();

};
