// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DeleteWorldWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDeleteWorldOnDeleteButtonClickedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDeleteWorldOnCancelButtonClickedSignature);

UCLASS()
class SAVESYSTEM_API UDeleteWorldWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UDeleteWorldWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	
	void Open(const FString& InWorldName);

	FDeleteWorldOnDeleteButtonClickedSignature OnDeleteButtonClicked;
	FDeleteWorldOnCancelButtonClickedSignature OnCancelButtonClicked;

private:
	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* TitleTextBlock;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* WarningTextBlock;

	UPROPERTY(Meta = (BindWidget))
	class UButton* DeleteButton;

	UPROPERTY(Meta = (BindWidget))
	class UButton* CancelButton;

	UPROPERTY()
	FString WorldName;

	UFUNCTION()
	void DeleteWorld();

	void BroadcastDeleteButtonClicked();

	UFUNCTION()
	void BroadcastCancelButtonClicked();

};
