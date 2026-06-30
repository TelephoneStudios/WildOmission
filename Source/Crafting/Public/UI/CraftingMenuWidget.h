// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Structs/ItemData.h"
#include "CraftingMenuWidget.generated.h"


UCLASS()
class CRAFTING_API UCraftingMenuWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UCraftingMenuWidget(const FObjectInitializer& ObjectInializer);
	
	virtual void NativeConstruct() override;

	UFUNCTION()
	void Refresh();

	void SetCategoryFilter(TEnumAsByte<EItemCategory> NewCategory);
	TEnumAsByte<EItemCategory> GetCategoryFilter() const;

	void SetSelectedRecipe(const FName& SelectedRecipeName);
	FName GetSelectedRecipe() const;

private:
	UPROPERTY(Meta = (BindWidget))
	class UCraftingRecipeCategoryButton* SortByButton_All;
	UPROPERTY(Meta = (BindWidget))
	class UCraftingRecipeCategoryButton* SortByButton_Items;
	UPROPERTY(Meta = (BindWidget))
	class UCraftingRecipeCategoryButton* SortByButton_Resources;
	UPROPERTY(Meta = (BindWidget))
	class UCraftingRecipeCategoryButton* SortByButton_Tools;
	UPROPERTY(Meta = (BindWidget))
	class UCraftingRecipeCategoryButton* SortByButton_Building;
	UPROPERTY(Meta = (BindWidget))
	class UCraftingRecipeCategoryButton* SortByButton_Weapons;
	UPROPERTY(Meta = (BindWidget))
	class UCraftingRecipeCategoryButton* SortByButton_Ammo;
	UPROPERTY(Meta = (BindWidget))
	class UCraftingRecipeCategoryButton* SortByButton_Medical;
	UPROPERTY(Meta = (BindWidget))
	class UCraftingRecipeCategoryButton* SortByButton_Clothing;

	UPROPERTY(Meta = (BindWidget))
	class UWrapBox* RecipesWrapBox;
	
	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* SelectedRecipeNameTextBlock;
	
	UPROPERTY(Meta = (BindWidget))
	class UImage* SelectedRecipeIconImage;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* SelectedRecipeYieldTextBlock;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* SelectedRecipeDescriptionTextBlock;

	UPROPERTY(Meta = (BindWidget))
	class UVerticalBox* IngredientListBox;

	UPROPERTY(Meta = (BindWidget))
	class UButton* CraftButton;

	UPROPERTY()
	TSubclassOf<class URecipeIconWidget> RecipeIconWidgetClass;
	
	UPROPERTY()
	TSubclassOf<class UIngredientRowWidget> IngredientRowWidgetClass;

	TEnumAsByte<EItemCategory> CategoryFilter;

	FName SelectedRecipe;

	void RefreshRecipesList();

	void RefreshDetailsPanel();
	void ClearDetailsPanel();
	void RefreshIngredientList();

	bool CanCraftRecipe(const FName& RecipeName);
	int32 GetRecipeIngredientPercentage(const FName& RecipeName);

	UFUNCTION()
	void Craft();

};
