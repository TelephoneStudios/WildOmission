// Copyright Telephone Studios. All Rights Reserved.


#include "UI/CreativeMenuWidget.h"
#include "Components/WrapBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Button.h"
#include "ItemCategoryButton.h"
#include "ItemIconWidget.h"
#include "Components/InventoryComponent.h"
#include "UObject/ConstructorHelpers.h"

UCreativeMenuWidget::UCreativeMenuWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	//ConstructorHelpers::FClassFinder<UItemIconWidget> ItemIconWidgetBlueprint(TEXT("/Game/CreativeMode/UI/WBP_ItemIcon"));

	//if (ItemIconWidgetBlueprint.Class == nullptr)
	//{
		//return;
	//}

	//ItemIconWidgetClass = ItemIconWidgetBlueprint.Class;
}

void UCreativeMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	CategoryFilter = EItemCategory::Item;

	SortByButton_All->Setup(this, EItemCategory::All);
	SortByButton_Items->Setup(this, EItemCategory::Item);
	SortByButton_Resources->Setup(this, EItemCategory::Resource);
	SortByButton_Tools->Setup(this, EItemCategory::Tool);
	SortByButton_Building->Setup(this, EItemCategory::Building);
	SortByButton_Weapons->Setup(this, EItemCategory::Weapon);
	SortByButton_Ammo->Setup(this, EItemCategory::Ammo);
	SortByButton_Medical->Setup(this, EItemCategory::Medical);
	SortByButton_Clothing->Setup(this, EItemCategory::Clothing);

	GiveSingleButton->OnClicked.AddDynamic(this, &UCreativeMenuWidget::GiveItemSingle);
	GiveStackButton->OnClicked.AddDynamic(this, &UCreativeMenuWidget::GiveItemStack);
	
	SelectedItem = TEXT("");
}

void UCreativeMenuWidget::Refresh()
{
	RefreshItemList();
	RefreshDetailsPanel();
}

void UCreativeMenuWidget::SetCategoryFilter(TEnumAsByte<EItemCategory> NewCategory)
{
	CategoryFilter = NewCategory;
	RefreshItemList();
}

TEnumAsByte<EItemCategory> UCreativeMenuWidget::GetCategoryFilter() const
{
	return CategoryFilter;
}

void UCreativeMenuWidget::SetSelectedItem(const FName& SelectedItemName)
{
	SelectedItem = SelectedItemName;
	RefreshDetailsPanel();
}

FName UCreativeMenuWidget::GetSelectedItem() const
{
	return SelectedItem;
}

void UCreativeMenuWidget::RefreshItemList()
{
	/*TArray<FCraftingRecipeEntry> RecipeEntries;
	for (const FName& RecipeID : UCraftingComponent::GetAllRecipes())
	{
		FItemData* YieldItemData = UInventoryComponent::GetItemData(RecipeID);
		if (YieldItemData == nullptr || (CategoryFilter != EItemCategory::All && YieldItemData->Category != CategoryFilter))
		{
			continue;
		}

		FCraftingRecipeEntry Entry;
		Entry.RecipeID = RecipeID;
		Entry.CanCraft = CanCraftRecipe(RecipeID);
		Entry.IngredientPercentage = GetRecipeIngredientPercentage(RecipeID);
		Entry.YieldItemData = UInventoryComponent::GetItemData(RecipeID);

		RecipeEntries.Add(Entry);
	}
	
	RecipeEntries.Sort();

	RecipesWrapBox->ClearChildren();
	for (int32 i = 0; i < RecipeEntries.Num(); i++)
	{
		const FCraftingRecipeEntry& RecipeEntry = RecipeEntries[i];

		URecipeIconWidget* NewRecipeIcon = CreateWidget<URecipeIconWidget>(this, RecipeIconWidgetClass);
		if (NewRecipeIcon == nullptr)
		{
			continue;
		}

		NewRecipeIcon->Setup(this, RecipeEntry);
		RecipesWrapBox->AddChild(NewRecipeIcon);
	}*/

}

void UCreativeMenuWidget::RefreshDetailsPanel()
{
	//if (SelectedRecipe == FName())
	//{
	//	ClearDetailsPanel();
	//	return;
	//}

	//FCraftingRecipe* RecipeData = UCraftingComponent::GetRecipe(SelectedRecipe);
	//if (RecipeData == nullptr)
	//{
	//	return;
	//}

	//FItemData* RecipeYieldItemData = UInventoryComponent::GetItemData(SelectedRecipe);
	//if (RecipeYieldItemData == nullptr)
	//{
	//	return;
	//}

	//SelectedRecipeNameTextBlock->SetText(FText::FromString(RecipeYieldItemData->DisplayName));
	//
	//SelectedRecipeDescriptionTextBlock->SetText(FText::FromString(RecipeYieldItemData->Description));

	//SelectedRecipeIconImage->SetBrushFromMaterial(RecipeYieldItemData->Thumbnail);
	//SelectedRecipeIconImage->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
	//
	//FString RecipeYieldQuantityString;

	//if (RecipeData->YieldQuantity > 1)
	//{
	//	RecipeYieldQuantityString = FString::Printf(TEXT("x%i"), RecipeData->YieldQuantity);
	//}

	//SelectedRecipeYieldTextBlock->SetText(FText::FromString(RecipeYieldQuantityString));

	//RefreshIngredientList();

	//CraftButton->SetIsEnabled(CanCraftRecipe(SelectedRecipe));
}

void UCreativeMenuWidget::ClearDetailsPanel()
{
	//SelectedRecipeNameTextBlock->SetText(FText::FromString(FString()));
	//
	//SelectedRecipeDescriptionTextBlock->SetText(FText::FromString(FString()));

	//SelectedRecipeIconImage->SetColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f));

	//CraftButton->SetIsEnabled(false);

	//IngredientListBox->ClearChildren();
}

void UCreativeMenuWidget::GiveItemSingle()
{
	/*UCraftingComponent* OwnerCraftingComponent = GetOwningPlayerPawn()->FindComponentByClass<UCraftingComponent>();
	if (OwnerCraftingComponent == nullptr)
	{
		return;
	}
	
	OwnerCraftingComponent->Server_CraftItem(SelectedRecipe);*/
}

void UCreativeMenuWidget::GiveItemStack()
{

}