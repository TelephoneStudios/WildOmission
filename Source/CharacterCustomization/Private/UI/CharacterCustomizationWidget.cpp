// Copyright Telephone Studios. All Rights Reserved.


#include "UI/CharacterCustomizationWidget.h"
#include "Components/Button.h"
#include "OptionBoxes/MultiOptionBox.h"
#include "Components/CustomCharacterComponent.h"
#include "Actors/CustomCharacter.h"
#include "Kismet/GameplayStatics.h"

UCharacterCustomizationWidget::UCharacterCustomizationWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UCharacterCustomizationWidget::NativeConstruct()
{
	Super::NativeConstruct();
	GenderMultiOptionBox->AddOption(TEXT("Male"));
	GenderMultiOptionBox->AddOption(TEXT("Female"));
	GenderMultiOptionBox->OnValueChangedNoParams.AddDynamic(this, &UCharacterCustomizationWidget::OnSettingChange);

	BackButton->OnClicked.AddDynamic(this, &UCharacterCustomizationWidget::OnBackClicked);
}

void UCharacterCustomizationWidget::OnOpen()
{
	FCustomCharacterData LoadedData = UCustomCharacterComponent::LoadData();

	// colors
	GenderMultiOptionBox->SetSelectedOption(LoadedData.bIsFemale ? TEXT("Female") : TEXT("Male"));
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
