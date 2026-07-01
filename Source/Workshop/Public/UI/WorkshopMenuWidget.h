// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WorkshopManager.h"
#include "WorkshopMenuWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorkshopMenuBackButtonClickedSignature);

UCLASS()
class WORKSHOP_API UWorkshopMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UWorkshopMenuWidget(const FObjectInitializer& ObjectInitializer);
	
	void OnOpen();
	void Refresh();

	virtual void NativeConstruct() override;
	
	FOnWorkshopMenuBackButtonClickedSignature OnBackButtonClicked;

protected:
	virtual void NativeTick(const FGeometry& MyGeomotry, float InDeltaTime) override;

private:
	UPROPERTY(Meta = (BindWidget))
	class UWidgetSwitcher* MenuSwitcher;

	UPROPERTY(Meta = (BindWidget))
	class UWrapBox* WorkshopItemsWrapBox;

	UPROPERTY(Meta = (BindWidget))
	class UButton* UploadButton;

	UPROPERTY(Meta = (BindWidget))
	class UButton* BackButton;

	UPROPERTY(Meta = (BindWidget))
	class UWidget* WorkshopMenu;

	UPROPERTY(Meta = (BindWidget))
	class UWidget* UploadingMenu;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* UploadProgressTextBlock;

	UPROPERTY(Meta = (BindWidget))
	class UProgressBar* UploadProgressBar;

	UPROPERTY(Meta = (BindWidget))
	class UWorldSelectionWidget* WorldSelectionMenu;

	UPROPERTY(Meta = (BindWidget))
	class UWorkshopUploadMenuWidget* UploadWorldMenu;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UWorkshopItemWidget> WorkshopItemClass;

	UPROPERTY()
	uint64 ItemUploadHandle;

	FTimerHandle CheckUploadStatusTimerHandle;

	TOptional<FString> SelectedFileID;

	UFUNCTION()
	void SelectWorkshopItem(const FString& FileID);

	UFUNCTION()
	void BackButtonClicked();

	UFUNCTION()
	void OnQueryCompleted(bool bSuccess, const TArray<FSteamWorkshopItemDetails>& Items);

	UFUNCTION()
	void OnUploadSubmitted();

	UFUNCTION()
	void OpenWorkshopMenu();

	UFUNCTION()
	void OpenWorldSelectionForUploading();

	UFUNCTION()
	void OpenUploadWorldMenu();

	UFUNCTION()
	void UploadWorld(const FString& WorldName, const FString& WorkshopItemName, const FString& WorkshopItemDescription);
};
