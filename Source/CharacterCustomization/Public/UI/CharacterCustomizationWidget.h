// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterCustomizationWidget.generated.h"

UCLASS()
class CHARACTERCUSTOMIZATION_API UCharacterCustomizationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UCharacterCustomizationWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;

private:
};
