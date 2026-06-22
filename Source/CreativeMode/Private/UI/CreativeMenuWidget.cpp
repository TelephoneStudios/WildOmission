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
	ConstructorHelpers::FClassFinder<UItemIconWidget> ItemIconWidgetBlueprint(TEXT("/Game/CreativeMode/UI/WBP_ItemIcon"));

	if (ItemIconWidgetBlueprint.Class == nullptr)
	{
		return;
	}

	ItemIconWidgetClass = ItemIconWidgetBlueprint.Class;
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
	TArray<FName> ItemList;
	for (const FName& ItemID : UInventoryComponent::GetAllItems())
	{
		FItemData* ItemData = UInventoryComponent::GetItemData(ItemID);
		if (ItemData == nullptr || (CategoryFilter != EItemCategory::All && ItemData->Category != CategoryFilter))
		{
			continue;
		}
		
		ItemList.Add(ItemID);
	}
	
	//ItemList.Sort();

	ItemsWrapBox->ClearChildren();
	for (int32 i = 0; i < ItemList.Num(); i++)
	{
		const FName& ItemID = ItemList[i];

		UItemIconWidget* NewItemIcon = CreateWidget<UItemIconWidget>(this, ItemIconWidgetClass);
		if (NewItemIcon == nullptr)
		{
			continue;
		}

		NewItemIcon->Setup(this, ItemID);
		ItemsWrapBox->AddChild(NewItemIcon);
	}
}

void UCreativeMenuWidget::RefreshDetailsPanel()
{
	if (SelectedItem == FName())
	{
		ClearDetailsPanel();
		return;
	}

	FItemData* SelectedItemData = UInventoryComponent::GetItemData(SelectedItem);
	if (SelectedItemData == nullptr)
	{
		return;
	}

	SelectedItemNameTextBlock->SetText(FText::FromString(SelectedItemData->DisplayName));
	
	SelectedItemDescriptionTextBlock->SetText(FText::FromString(SelectedItemData->Description));

	SelectedItemIconImage->SetBrushFromMaterial(SelectedItemData->Thumbnail);
	SelectedItemIconImage->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));

	const FString StackButtonString = FString::Printf(TEXT("Give Stack (x%i)"), SelectedItemData->StackSize);
	GiveStackButtonTextBlock->SetText(FText::FromString(StackButtonString));
}

void UCreativeMenuWidget::ClearDetailsPanel()
{
	SelectedItemNameTextBlock->SetText(FText::FromString(FString()));
	
	SelectedItemDescriptionTextBlock->SetText(FText::FromString(FString()));

	SelectedItemIconImage->SetColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f));}

void UCreativeMenuWidget::GiveItemSingle()
{
	if (!GetOwningPlayerPawn())
	{
		return;
	}

	UInventoryComponent* OwnerInventoryComponent = GetOwningPlayerPawn()->GetComponentByClass<UInventoryComponent>();
	if (OwnerInventoryComponent == nullptr)
	{
		return;
	}

	FInventoryItem ItemToAdd;
	ItemToAdd.Name = SelectedItem;
	ItemToAdd.Quantity = 1;

	OwnerInventoryComponent->AddItem(ItemToAdd);
}

void UCreativeMenuWidget::GiveItemStack()
{
	if (!GetOwningPlayerPawn())
	{
		return;
	}

	UInventoryComponent* OwnerInventoryComponent = GetOwningPlayerPawn()->GetComponentByClass<UInventoryComponent>();
	if (OwnerInventoryComponent == nullptr)
	{
		return;
	}

	FItemData* SelectedItemData = UInventoryComponent::GetItemData(SelectedItem);
	if (SelectedItemData == nullptr)
	{
		return;
	}

	FInventoryItem ItemToAdd;
	ItemToAdd.Name = SelectedItem;
	ItemToAdd.Quantity = SelectedItemData->StackSize;

	OwnerInventoryComponent->AddItem(ItemToAdd);
}