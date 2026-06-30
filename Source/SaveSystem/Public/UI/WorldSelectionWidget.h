// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WorldSelectionWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSelectButtonClickedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCancelButtonClickedSignature);

UCLASS()
class SAVESYSTEM_API UWorldSelectionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UWorldSelectionWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	void SetWorldList(const TArray<FString>& WorldNames);
	TOptional<FString> SelectedWorldName;

	FOnSelectButtonClickedSignature OnSelectButtonClicked;
	FOnCancelButtonClickedSignature OnCancelButtonClicked;
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY(Meta = (BindWidget))
	UPanelWidget* WorldListBox;

	UPROPERTY(Meta = (BindWidget))
	class UButton* SelectButton;

	UPROPERTY(Meta = (BindWidget))
	class UButton* CancelButton;

	TSubclassOf<class UWorldRowWidget> WorldRowWidgetClass;
	
	void UpdateListChildren();

	TArray<class UWorldInformation*> GetWorldsSortedByLastPlayed(const TArray<FString>& NameList);
	static bool IsSaveMoreRecentlyPlayed(class UWorldInformation* WorldA, class UWorldInformation* WorldB);

	UFUNCTION()
	void SetSelectedWorld(const FString& WorldName);

	UFUNCTION()
	void BroadcastSelectButtonClicked();

	UFUNCTION()
	void BroadcastCancelButtonClicked();

};
