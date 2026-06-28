// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WorkshopMenuWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorkshopMenuBackButtonClickedSignature);

UCLASS()
class WORKSHOP_API UWorkshopMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UWorkshopMenuWidget(const FObjectInitializer& ObjectInitializer);
	
	virtual void NativeConstruct() override;
	
	FOnWorkshopMenuBackButtonClickedSignature OnBackButtonClicked;

private:
	UPROPERTY(Meta = (BindWidget))
	class UWidgetSwitcher* MenuSwitcher;

	UPROPERTY(Meta = (BindWidget))
	class UButton* UploadButton;

	UPROPERTY(Meta = (BindWidget))
	class UButton* BackButton;

	UPROPERTY(Meta = (BindWidget))
	class UWidget* BrowseMenu;

	UPROPERTY(Meta = (BindWidget))
	class UWidget* UploadingMenu;

	UPROPERTY()
	uint64 ItemUploadHandle;

	UFUNCTION()
	void OnUploadButtonClicked();

	UFUNCTION()
	void BackButtonClicked();

	UFUNCTION()
	void CheckUploadStatus();
};
