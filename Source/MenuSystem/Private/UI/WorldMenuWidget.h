// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enums/GameDifficulty.h"
#include "WorldMenuWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FWorldMenuOnPlayButtonClickedSignature, const FString&, WorldName, const FString&, ServerName, const bool, IsMultiplayer, const bool, IsFriendsOnly, const uint8&, GameMode, const int32&, MaxPlayerCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWorldMenuOnRenameButtonClickedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWorldMenuOnDeleteButtonClickedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWorldMenuOnCancelButtonClickedSignature);

UCLASS()
class UWorldMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UWorldMenuWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	void Open(const FString& InWorldName);

	FWorldMenuOnPlayButtonClickedSignature OnPlayButtonClicked;
	FWorldMenuOnRenameButtonClickedSignature OnRenameButtonClicked;
	FWorldMenuOnDeleteButtonClickedSignature OnDeleteButtonClicked;
	FWorldMenuOnCancelButtonClickedSignature OnCancelButtonClicked;

private:
	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* Title;
	
	UPROPERTY(Meta = (BindWidget))
	class UButton* PlayButton;
	
	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* PlayButtonTextBlock;

	UPROPERTY(Meta = (BindWidget))
	class UButton* RenameButton;

	UPROPERTY(Meta = (BindWidget))
	class UButton* DeleteButton;

	UPROPERTY(Meta = (BindWidget))
	class UButton* CancelButton;
	
	UPROPERTY(Meta = (BindWidget))
	class UMultiOptionBox* DifficultyMultiOptionBox;

	UPROPERTY(Meta = (BindWidget))
	class UMultiOptionBox* GameModeMultiOptionBox;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* SeedTextBlock;

	UPROPERTY(Meta = (BindWidget))
	class UCheckOptionBox* MultiplayerCheckOptionBox;
	
	UPROPERTY(Meta = (BindWidget))
	class UCheckOptionBox* FriendsOnlyCheckOptionBox;
	
	UPROPERTY(Meta = (BindWidget))
	class USliderOptionBox* MaxPlayersSliderOptionBox;

	UPROPERTY(Meta = (BindWidget))
	UWidget* HostSettingsMenu;
	
	UPROPERTY(Meta = (BindWidget))
	class UEditableTextBox* ServerNameInputBox;

	UPROPERTY()
	FString WorldName;

	int32 GetWorldVersion() const;

	TEnumAsByte<enum EGameDifficulty> GetWorldDifficulty() const;
	void SetWorldDifficultyAndGameMode(const TEnumAsByte<enum EGameDifficulty>& NewDifficulty, const uint8& NewGameMode);
	
	UFUNCTION()
	void ServerNameOnTextChanged(const FText& Text);
	
	UFUNCTION()
	void OnGameModeSelectionChanged(const FString& NewSelection);

	UFUNCTION()
	void MultiplayerCheckboxChanged(bool bIsChecked);

	UFUNCTION()
	void BroadcastPlayButtonClicked();

	UFUNCTION()
	void BroadcastRenameButtonClicked() const;

	UFUNCTION()
	void BroadcastDeleteButtonClicked() const;

	UFUNCTION()
	void BroadcastCancelButtonClicked() const;

};
