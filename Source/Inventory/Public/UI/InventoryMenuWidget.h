// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryMenuWidget.generated.h"

UCLASS()
class INVENTORY_API UInventoryMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UInventoryMenuWidget(const FObjectInitializer& ObjectInitializer);


	UFUNCTION(BlueprintCallable)
	void Open();
	UFUNCTION(BlueprintCallable)
	void Close(bool ShouldCloseContainer = false);

	UFUNCTION(BlueprintCallable)
	bool IsOpen() const;
	UFUNCTION(BlueprintCallable)
	class UPlayerInventoryWidget* GetPlayerInventoryWidget() const;
	UFUNCTION(BlueprintCallable)
	class USelectedItemWidget* GetSelectedItemWidget() const;
	UFUNCTION(BlueprintCallable)
	class UHoveredItemNameTag* GetHoveredItemNameTagWidget() const;
	UFUNCTION(BlueprintCallable)
	bool SelectedItemVisible() const;
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeomentry, float InDeltaTime) override;

private:
	UPROPERTY(Meta = (BindWidget))
	class UHorizontalBox* InventoryHorizontalBox;
	
	UPROPERTY(Meta = (BindWidget))
	class UPlayerInventoryWidget* PlayerInventoryWidget;

	UPROPERTY()
	class UInventoryWidget* OpenContainerWidget;

	UPROPERTY(Meta = (BindWidget))
	class USelectedItemWidget* SelectedItemWidget;

	UPROPERTY(Meta = (BindWidget))
	class UHoveredItemNameTag* HoveredItemNameTagWidget;

	UPROPERTY(Meta = (BindWidget))
	class UPanelWidget* ClothingInventoryPanel;

	UFUNCTION()
	void OnOpenContainerChanged(class UInventoryComponent* OpenContainer);

	void CreateOpenContainerWidget(class UInventoryComponent* OpenContainer);
	void DestroyOpenContainerWidget();

	void UpdateFollowMousePointerWidgets();

};
