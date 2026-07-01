// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WorkshopManager.h"
#include "WorkshopItemWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorkshopItemButtonClickedSignature, const FString&, FileID);

UCLASS()
class WORKSHOP_API UWorkshopItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UWorkshopItemWidget(const FObjectInitializer& ObjectInitializer);
	
	virtual void NativeConstruct() override;

	void Setup(const FSteamWorkshopItemDetails& Details);

	FOnWorkshopItemButtonClickedSignature OnClicked;

protected:
	virtual void NativeTick(const FGeometry& MyGeomotry, float InDeltaTime) override;

private:
	UPROPERTY(Meta = (BindWidget))
	class UButton* Button;
	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* NameTextBlock;
	UPROPERTY(Meta = (BindWidget))
	class UImage* PreviewImage;

	FString FileID;

	UFUNCTION()
	void OnButtonClicked();

};
