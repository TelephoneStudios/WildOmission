// Copyright Telephone Studios. All Rights Reserved.


#include "ItemCategoryButton.h"
#include "UI/CreativeMenuWidget.h"
#include "Structs/ItemData.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Color/UIColors.h"

UItemCategoryButton::UItemCategoryButton(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	ParentCreativeMenu = nullptr;
	Category = EItemCategory::Item;
}

void UItemCategoryButton::Setup(UCreativeMenuWidget* InParent, TEnumAsByte<EItemCategory> InCategory)
{
	Button->OnClicked.AddDynamic(this, &UItemCategoryButton::OnClicked);
	
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

	ParentCreativeMenu = InParent;
	Category = InCategory;
}

void UItemCategoryButton::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	//if (ParentCraftingMenu && ParentCraftingMenu->GetCategoryFilter() == Category)
	//{
	//	FUIColor* Blue = UUIColors::GetBaseColor(TEXT("Blue"));
	//	Button->SetBackgroundColor(Blue->Default * FLinearColor(1.0f, 1.0f, 1.0f, 0.5f));
	//}
	//else
	//{
	//	FUIColor* LightGray = UUIColors::GetBaseColor(TEXT("LightGray"));
	//	Button->SetBackgroundColor(LightGray->Default * FLinearColor(1.0f, 1.0f, 1.0f, 0.5f));
	//}
}

void UItemCategoryButton::OnClicked()
{
	if (ParentCreativeMenu == nullptr)
	{
		return;
	}

	ParentCreativeMenu->SetCategoryFilter(Category);
}