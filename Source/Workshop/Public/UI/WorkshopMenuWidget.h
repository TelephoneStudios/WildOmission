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
	
	UFUNCTION()
	void OnOpen();
	UFUNCTION()
	void Refresh();

	virtual void NativeConstruct() override;
	
	FOnWorkshopMenuBackButtonClickedSignature OnBackButtonClicked;

protected:
	virtual void NativeTick(const FGeometry& MyGeomotry, float InDeltaTime) override;

private:
	// Workshop menu
	UPROPERTY(Meta = (BindWidget))
	class UWidgetSwitcher* MenuSwitcher;
	UPROPERTY(Meta = (BindWidget))
	class UWrapBox* WorkshopItemsWrapBox;
	UPROPERTY(Meta = (BindWidget))
	class UButton* RefreshButton;
	UPROPERTY(Meta = (BindWidget))
	class UButton* UploadButton;
	UPROPERTY(Meta = (BindWidget))
	class UButton* BackButton;
	UPROPERTY(Meta = (BindWidget))
	class UWidget* WorkshopMenu;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UWorkshopItemWidget> WorkshopItemClass;

	// Loading menu
	UPROPERTY(Meta = (BindWidget))
	class UWidget* LoadingMenu;
	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* LoadingMenuTitleTextBlock;
	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* LoadingProgressTextBlock;
	UPROPERTY(Meta = (BindWidget))
	class UProgressBar* LoadingProgressBar;

	// World uploading
	UPROPERTY(Meta = (BindWidget))
	class UWorldSelectionWidget* WorldSelectionMenu;
	UPROPERTY(Meta = (BindWidget))
	class UWorkshopUploadMenuWidget* UploadWorldMenu;

	// Details panel
	UPROPERTY(Meta = (BindWidget))
	class UWidget* SelectedItemDetailsPanel;
	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* SelectedItemTitleTextBlock;
	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* SelectedItemDescriptionTextBlock;
	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* SelectedItemLikesTextBlock;
	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* SelectedItemDislikesTextBlock;
	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* SelectedItemSubscribersTextBlock;
	UPROPERTY(Meta = (BindWidget))
	class UImage* SelectedItemImage;
	UPROPERTY(Meta = (BindWidget))
	class UButton* OpenWorkshopPageForSelectedButton;
	UPROPERTY(Meta = (BindWidget))
	class UButton* DownloadSelectedWorldButton;
	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* DownloadButtonTextBlock;

	UPROPERTY()
	uint64 ItemUploadHandle;

	FTimerHandle CheckUploadStatusTimerHandle;

	TOptional<FSteamWorkshopItemDetails> SelectedItem;

	UFUNCTION()
	void SelectWorkshopItem(const FSteamWorkshopItemDetails& Details);

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

	UFUNCTION()
	void OpenWorkshopForSelectedWorld();

	UFUNCTION()
	void DownloadWorld();
};
