// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WorldRowWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorldRowWidgetClickedSignature, const FString&, SelectedWorldName);

USTRUCT()
struct FWorldRowInformation
{
	GENERATED_BODY()

	UPROPERTY()
	UTexture2D* Icon;

	UPROPERTY()
	uint32 DaysPlayed;

	UPROPERTY()
	uint8 CreationMonth;

	UPROPERTY()
	uint8 CreationDay;

	UPROPERTY()
	uint16 CreationYear;

	UPROPERTY()
	FString Name;

	FWorldRowInformation();
	FWorldRowInformation(class UTexture2D* InIcon, const uint32& InDaysPlayed, const uint8& InCreationMonth, const uint8& InCreationDay, const uint16& InCreationYear, const FString& InName);
};

UCLASS()
class UWorldRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UWorldRowWidget(const FObjectInitializer& ObjectIntializer);

	virtual void NativeConstruct() override;

	void Setup(const FWorldRowInformation& InInformation);
	
	void SetSelected(bool InSelected);
	FString GetWorldName() const;

	FOnWorldRowWidgetClickedSignature OnClicked;

protected:	
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
private:
	UPROPERTY(Meta = (BindWidget))
	class UImage* Icon;
	
	UPROPERTY(Meta = (BindWidget))
	class UButton* RowButton;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* WorldNameTextBlock;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* DateCreatedTextBlock;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* DaysPlayedTextBlock;

	UPROPERTY()
	FString WorldName;

	UPROPERTY()
	bool Selected;

	UFUNCTION()
	void BroadcastOnClicked();

};
