// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Structs/ItemData.h"
#include "ItemCategoryButton.generated.h"


UCLASS()
class UItemCategoryButton : public UUserWidget
{
	GENERATED_BODY()

public:	
	UItemCategoryButton(const FObjectInitializer& ObjectInitializer);

	void Setup(class UCreativeMenuWidget* InParent, TEnumAsByte<EItemCategory> InCategory);
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY(Meta = (BindWidget))
	class UButton* Button;
	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* TextBlock;
	
	UPROPERTY()
	class UCreativeMenuWidget* ParentCreativeMenu;
	
	UPROPERTY()
	TEnumAsByte<EItemCategory> Category;

	UFUNCTION()
	void OnClicked();

};
