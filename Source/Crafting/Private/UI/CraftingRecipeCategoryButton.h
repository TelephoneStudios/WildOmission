// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Structs/ItemData.h"
#include "CraftingRecipeCategoryButton.generated.h"

UCLASS()
class UCraftingRecipeCategoryButton : public UUserWidget
{
	GENERATED_BODY()

public:	
	UCraftingRecipeCategoryButton(const FObjectInitializer& ObjectInitializer);

	void Setup(class UCraftingMenuWidget* InParent, TEnumAsByte<EItemCategory> InCategory);
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY(Meta = (BindWidget))
	class UButton* Button;
	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* TextBlock;
	
	UPROPERTY()
	class UCraftingMenuWidget* ParentCraftingMenu;
	
	UPROPERTY()
	TEnumAsByte<EItemCategory> Category;

	UFUNCTION()
	void OnClicked();

};
