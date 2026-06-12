// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Structs/ItemData.h"
#include "CraftingRecipeCategoryButton.generated.h"

class UCraftingMenuWidget;
class UButton;
class UTextBlock;

UCLASS()
class UCraftingRecipeCategoryButton : public UUserWidget
{
	GENERATED_BODY()

public:	
	UCraftingRecipeCategoryButton(const FObjectInitializer& ObjectInitializer);

	void Setup(UCraftingMenuWidget* InParent, TEnumAsByte<EItemCategory> InCategory);
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY(Meta = (BindWidget))
	UButton* Button;
	UPROPERTY(Meta = (BindWidget))
	UTextBlock* TextBlock;
	
	UPROPERTY()
	UCraftingMenuWidget* ParentCraftingMenu;
	
	UPROPERTY()
	TEnumAsByte<EItemCategory> Category;

	UFUNCTION()
	void OnClicked();

};
