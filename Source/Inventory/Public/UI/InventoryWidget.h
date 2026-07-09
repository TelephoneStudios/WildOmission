// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySlotWidget.h"
#include "InventoryWidget.generated.h"

UCLASS()
class INVENTORY_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UInventoryWidget(const FObjectInitializer& ObjectInitializer);

	virtual void Setup(class UInventoryMenuWidget* InParentMenu, class UInventoryComponent* InInventoryComponent);

	UFUNCTION()
	virtual void Refresh();
	
	virtual void RefreshSlot(const int32& SlotIndex);

	void CreateSlots();

	UFUNCTION(BlueprintCallable)
	void Open();
	UFUNCTION(BlueprintCallable)
	void Close();

	UFUNCTION(BlueprintCallable)
	bool IsOpen() const;

	UFUNCTION(BlueprintCallable)
	class UInventoryMenuWidget* GetParentMenu() const;
	UFUNCTION(BlueprintCallable)
	class UInventoryComponent* GetInventoryComponent() const;

	UClass* GetSlotWidgetClass() const;

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* InventoryName;

	UPROPERTY(meta = (BindWidget))
	class UUniformGridPanel* InventoryGridPanel;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UInventorySlotWidget> SlotWidgetClass;

	UPROPERTY()
	TArray<class UInventorySlotWidget*> Slots;

private:
	UPROPERTY()
	class UInventoryMenuWidget* ParentMenu;

	UPROPERTY()
	class UInventoryComponent* InventoryComponent;

};
