// Copyright Telephone Studios. All Rights Reserved.


#include "CraftingRecipeCategoryButton.h"
#include "UI/CraftingMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Color/UIColors.h"

UCraftingRecipeCategoryButton::UCraftingRecipeCategoryButton(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	ParentCraftingMenu = nullptr;
	Category = EItemCategory::Item;
}

void UCraftingRecipeCategoryButton::Setup(UCraftingMenuWidget* InParent, TEnumAsByte<EItemCategory> InCategory)
{
	Button->OnClicked.AddDynamic(this, &UCraftingRecipeCategoryButton::OnClicked);
	
	FString ButtonTextString = TEXT("");
	switch (InCategory)
	{
	case All:
		ButtonTextString = TEXT("All");
		break;
	case Item:
		ButtonTextString = TEXT("Items");
		break;
	case Resource:
		ButtonTextString = TEXT("Resource");
		break;
	case Tool:
		ButtonTextString = TEXT("Tools");
		break;
	case Building:
		ButtonTextString = TEXT("Building");
		break;
	case Weapon:
		ButtonTextString = TEXT("Weapons");
		break;
	case Ammo:
		ButtonTextString = TEXT("Ammo");
		break;
	case Medical:
		ButtonTextString = TEXT("Medical");
		break;
	case Clothing:
		ButtonTextString = TEXT("Clothing");
		break;
	default:
		ButtonTextString = TEXT("Default");
		break;
	}
	
	TextBlock->SetText(FText::FromString(ButtonTextString));

	ParentCraftingMenu = InParent;
	Category = InCategory;
}

void UCraftingRecipeCategoryButton::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (ParentCraftingMenu && ParentCraftingMenu->GetCategoryFilter() == Category)
	{
		FUIColor* Blue = UUIColors::GetBaseColor(TEXT("Blue"));
		Button->SetBackgroundColor(Blue->Default * FLinearColor(1.0f, 1.0f, 1.0f, 0.5f));
	}
	else
	{
		FUIColor* LightGray = UUIColors::GetBaseColor(TEXT("LightGray"));
		Button->SetBackgroundColor(LightGray->Default * FLinearColor(1.0f, 1.0f, 1.0f, 0.5f));
	}
}

void UCraftingRecipeCategoryButton::OnClicked()
{
	if (ParentCraftingMenu == nullptr)
	{
		return;
	}

	ParentCraftingMenu->SetCategoryFilter(Category);
}