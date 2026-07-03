// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RenameWorldWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRenameWorldOnRenameButtonClickedSignature, const FString&, NewWorldName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRenameWorldOnCancelButtonClickedSignature);

UCLASS()
class SAVESYSTEM_API URenameWorldWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	URenameWorldWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	void Open(const FString& InWorld);

	FRenameWorldOnRenameButtonClickedSignature OnRenameButtonClicked;
	FRenameWorldOnCancelButtonClickedSignature OnCancelButtonClicked;

private:
	UPROPERTY(Meta = (BindWidget))
	class UEditableTextBox* WorldNameInputBox;
	
	UPROPERTY(Meta = (BindWidget))
	class UButton* RenameButton;

	UPROPERTY(Meta = (BindWidget))
	class UButton* CancelButton;

	UPROPERTY(Meta = (BindWidget))
	UWidget* InvalidWarningBorder;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* InvalidWarningTextBlock;

	UPROPERTY()
	FString World;

	UFUNCTION()
	void RenameWorld();

	UFUNCTION()
	void ShowInvalidWarning(const FString& Warning);

	UFUNCTION()
	void HideInvalidWarning();

	bool WorldOfSameNameAlreadyExists(const FString& WorldName);
	bool WorldContainsInvalidCharacter(const FString& WorldName);

	void BroadcastRenameButtonClicked(const FString& NewWorldName);

	UFUNCTION()
	void BroadcastCancelButtonClicked();

	UFUNCTION()
	void WorldNameOnTextChanged(const FText& Text);

};
