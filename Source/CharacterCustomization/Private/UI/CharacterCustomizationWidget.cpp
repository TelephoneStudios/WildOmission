// Copyright Telephone Studios. All Rights Reserved.


#include "UI/CharacterCustomizationWidget.h"
#include "Components/Button.h"
#include "OptionBoxes/MultiOptionBox.h"
#include "OptionBoxes/ColorOptionBox.h"
#include "Components/CustomCharacterComponent.h"
#include "Actors/CustomCharacter.h"
#include "Kismet/GameplayStatics.h"

UCharacterCustomizationWidget::UCharacterCustomizationWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	SetIsFocusable(true);

	GenderMultiOptionBox = nullptr;
	SkinColorOptionBox = nullptr;
	ShirtColorOptionBox = nullptr;
	PantsColorOptionBox = nullptr;
	ShoeColorOptionBox = nullptr;
}

void UCharacterCustomizationWidget::NativeConstruct()
{
	Super::NativeConstruct();
	GenderMultiOptionBox->AddOption(TEXT("Male"));
	GenderMultiOptionBox->AddOption(TEXT("Female"));
	GenderMultiOptionBox->OnValueChangedNoParams.AddDynamic(this, &UCharacterCustomizationWidget::OnSettingChange);
	SkinColorOptionBox->OnColorChangedNoParams.AddDynamic(this, &UCharacterCustomizationWidget::OnSettingChange);
	ShirtColorOptionBox->OnColorChangedNoParams.AddDynamic(this, &UCharacterCustomizationWidget::OnSettingChange);
	PantsColorOptionBox->OnColorChangedNoParams.AddDynamic(this, &UCharacterCustomizationWidget::OnSettingChange);
	ShoeColorOptionBox->OnColorChangedNoParams.AddDynamic(this, &UCharacterCustomizationWidget::OnSettingChange);
	BackButton->OnClicked.AddDynamic(this, &UCharacterCustomizationWidget::OnBackClicked);
}

void UCharacterCustomizationWidget::OnOpen()
{
	FCustomCharacterData LoadedData = UCustomCharacterComponent::LoadData();

	// Gender
	GenderMultiOptionBox->SetSelectedOption(LoadedData.bIsFemale ? TEXT("Female") : TEXT("Male"));

	// Colors
	SkinColorOptionBox->SetColor(LoadedData.SkinColor);
	ShirtColorOptionBox->SetColor(LoadedData.ShirtColor);
	PantsColorOptionBox->SetColor(LoadedData.PantsColor);
	ShoeColorOptionBox->SetColor(LoadedData.ShoeColor);

}

void UCharacterCustomizationWidget::OnSettingChange()
{
	// todo apply it to the character
	ACustomCharacter* CustomCharacterPreview = Cast<ACustomCharacter>(UGameplayStatics::GetActorOfClass(GetWorld(), ACustomCharacter::StaticClass()));
	if (CustomCharacterPreview == nullptr)
	{
		return;
	}

	UCustomCharacterComponent* CustomComp = CustomCharacterPreview->GetComponentByClass<UCustomCharacterComponent>();
	if (CustomComp == nullptr)
	{
		return;
	}
	FCustomCharacterData Data;
	Data.bIsFemale = GenderMultiOptionBox->GetSelectedOption() == TEXT("Female");
	Data.SkinColor = SkinColorOptionBox->GetSelectedColor();
	Data.ShirtColor = ShirtColorOptionBox->GetSelectedColor();
	Data.PantsColor = PantsColorOptionBox->GetSelectedColor();
	Data.ShoeColor = ShoeColorOptionBox->GetSelectedColor();
	CustomComp->Apply(Data);

	UCustomCharacterComponent::SaveData(Data);
}

void UCharacterCustomizationWidget::OnBackClicked()
{
	if (OnBackButtonClicked.IsBound())
	{
		OnBackButtonClicked.Broadcast();
	}
}
