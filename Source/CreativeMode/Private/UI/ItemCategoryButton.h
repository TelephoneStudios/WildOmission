// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemCategoryButton.generated.h"

class UCreativeMenuWidget;
class UButton;
class UTextBlock;
enum EItemCategory;

UCLASS()
class UItemCategoryButton : public UUserWidget
{
	GENERATED_BODY()

public:	
	UItemCategoryButton(const FObjectInitializer& ObjectInitializer);

	void Setup(UCreativeMenuWidget* InParent, TEnumAsByte<EItemCategory> InCategory);
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY(Meta = (BindWidget))
	UButton* Button;
	UPROPERTY(Meta = (BindWidget))
	UTextBlock* TextBlock;
	
	UPROPERTY()
	UCreativeMenuWidget* ParentCreativeMenu;
	
	UPROPERTY()
	TEnumAsByte<EItemCategory> Category;

	UFUNCTION()
	void OnClicked();

};
