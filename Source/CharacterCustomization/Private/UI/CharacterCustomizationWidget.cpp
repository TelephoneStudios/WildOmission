// Copyright Telephone Studios. All Rights Reserved.


#include "UI/CharacterCustomizationWidget.h"

UCharacterCustomizationWidget::UCharacterCustomizationWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UCharacterCustomizationWidget::NativeConstruct()
{
	Super::NativeConstruct();

}