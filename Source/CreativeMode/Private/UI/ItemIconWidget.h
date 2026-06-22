// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "ItemIconWidget.generated.h"

UCLASS()
class UItemIconWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
public:
	void Setup(class UCreativeMenuWidget* InParentMenu, const FName& InItemID);
	
	UFUNCTION(BlueprintCallable)
	bool IsSelected() const;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

private:
	UPROPERTY(Meta = (BindWidget))
	class UButton* ItemButton;
	
	UPROPERTY(Meta = (BindWidget))
	class UBorder* ItemIconBorder;

	UPROPERTY(Meta = (BindWidget))
	UBorder* BackgroundBorder;

	UPROPERTY()
	class UCreativeMenuWidget* ParentMenu;

	bool Hovering = false;
	FName ItemID;

	UFUNCTION()
	void OnClicked();
};
