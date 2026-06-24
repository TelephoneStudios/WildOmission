// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CreativeMenuWidget.generated.h"

UCLASS()
class CREATIVEMODE_API UCreativeMenuWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UCreativeMenuWidget(const FObjectInitializer& ObjectInializer);
	
	virtual void NativeConstruct() override;

	UFUNCTION()
	void Refresh();

	void SetCategoryFilter(TEnumAsByte<enum EItemCategory> NewCategory);
	TEnumAsByte<enum EItemCategory> GetCategoryFilter() const;

	void SetSelectedItem(const FName& SelectedItemName);
	FName GetSelectedItem() const;

private:
	UPROPERTY(Meta = (BindWidget))
	class UItemCategoryButton* SortByButton_All;
	UPROPERTY(Meta = (BindWidget))
	class UItemCategoryButton* SortByButton_Items;
	UPROPERTY(Meta = (BindWidget))
	class UItemCategoryButton* SortByButton_Resources;
	UPROPERTY(Meta = (BindWidget))
	class UItemCategoryButton* SortByButton_Tools;
	UPROPERTY(Meta = (BindWidget))
	class UItemCategoryButton* SortByButton_Building;
	UPROPERTY(Meta = (BindWidget))
	class UItemCategoryButton* SortByButton_Weapons;
	UPROPERTY(Meta = (BindWidget))
	class UItemCategoryButton* SortByButton_Ammo;
	UPROPERTY(Meta = (BindWidget))
	class UItemCategoryButton* SortByButton_Medical;
	UPROPERTY(Meta = (BindWidget))
	class UItemCategoryButton* SortByButton_Clothing;

	UPROPERTY(Meta = (BindWidget))
	class UWrapBox* ItemsWrapBox;
	
	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* SelectedItemNameTextBlock;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* GiveStackButtonTextBlock;
	
	UPROPERTY(Meta = (BindWidget))
	class UImage* SelectedItemIconImage;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* SelectedItemDescriptionTextBlock;

	UPROPERTY(Meta = (BindWidget))
	class UButton* GiveSingleButton;

	UPROPERTY(Meta = (BindWidget))
	class UButton* GiveStackButton;

	UPROPERTY(Meta = (BindWidget))
	class UButton* SwitchMenuButton;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* SwitchMenuButtonTextBlock;

	UPROPERTY(Meta = (BindWidget))
	class UWidgetSwitcher* MenuSwitcher;

	UPROPERTY(Meta = (BindWidget))
	class UWidget* ItemMenu;

	UPROPERTY(Meta = (BindWidget))
	class UWidget* CommandsMenu;

	UPROPERTY()
	TSubclassOf<class UItemIconWidget> ItemIconWidgetClass;
	
	TEnumAsByte<enum EItemCategory> CategoryFilter;

	FName SelectedItem;

	void RefreshItemList();

	void RefreshDetailsPanel();
	void ClearDetailsPanel();

	UFUNCTION()
	void SwitchMenu();

	UFUNCTION()
	void GiveItemSingle();

	UFUNCTION()
	void GiveItemStack();
};
