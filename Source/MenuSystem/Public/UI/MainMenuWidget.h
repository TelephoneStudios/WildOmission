// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Structs/ServerData.h"
#include "Interfaces/MenuInterface.h"
#include "MainMenuWidget.generated.h"

UCLASS()
class MENUSYSTEM_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMainMenuWidget(const FObjectInitializer& ObjectInitializer);
	
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void OpenMainMenu();
	UFUNCTION()
	void OpenWorldSelectionMenu();
	UFUNCTION()
	void OpenWorldCreationMenu();
	UFUNCTION()
	void OpenWorldMenu();
	UFUNCTION()
	void OpenWorldMenuForWorld(const FString& WorldName);
	UFUNCTION()
	void HostGame(const FString& WorldName, const FString& ServerName, const bool IsMultiplayer, const bool IsFriendsOnly, const int32& MaxPlayerCount);
	UFUNCTION()
	void OpenRenameWorldMenu();
	UFUNCTION()
	void OpenDeleteWorldMenu();
	UFUNCTION()
	void OpenServerBrowserMenu();
	UFUNCTION()
	void OpenOptionsMenu();
	UFUNCTION()
	void OpenFeedbackPage();
	UFUNCTION()
	void OpenCreditsMenu();
	UFUNCTION()
	void ExitGame();
	UFUNCTION()
	void OpenErrorPrompt(const FString& Title, const FString& Error);

	void SetServerList(TArray<FServerData> InServerData);

	void Setup(IMenuInterface* InMenuInterface);
	void Teardown();

	IMenuInterface* GetMenuInterface() const;

private:
	UPROPERTY(Meta = (BindWidget))
	class UWidgetSwitcher* MenuSwitcher;

	UPROPERTY(Meta = (BindWidget))
	class UButton* PlayButton;
	UPROPERTY(Meta = (BindWidget))
	class UButton* OptionsButton;
	UPROPERTY(Meta = (BindWidget))
	class UButton* FeedbackButton;
	UPROPERTY(Meta = (BindWidget))
	class UButton* CreditsButton;
	UPROPERTY(Meta = (BindWidget))
	class UButton* ExitButton;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* SplashTextBlock;

	UPROPERTY(Meta = (BindWidget))
	UWidget* MainMenu;
	UPROPERTY(Meta = (BindWidget))
	class UWorldSelectionWidget* WorldSelectionMenu;
	UPROPERTY(Meta = (BindWidget))
	class UWorldCreationWidget* WorldCreationMenu;
	UPROPERTY(Meta = (BindWidget))
	class UWorldMenuWidget* WorldMenu;
	UPROPERTY(Meta = (BindWidget))
	class URenameWorldWidget* RenameWorldMenu;
	UPROPERTY(Meta = (BindWidget))
	class UDeleteWorldWidget* DeleteWorldMenu;
	UPROPERTY(Meta = (BindWidget))
	class UServerBrowserWidget* ServerBrowserMenu;
	UPROPERTY(Meta = (BindWidget))
	class UOptionsWidget* OptionsMenu;
	UPROPERTY(Meta = (BindWidget))
	class UCreditsWidget* CreditsMenu;
	UPROPERTY(Meta = (BindWidget))
	class UErrorMessagePrompt* ErrorMessagePrompt;

	class IMenuInterface* MenuInterface;
	
	UFUNCTION()
	void RefreshSplashText();

	UFUNCTION()
	void JoinServer(const uint32& ServerIndex);

	UFUNCTION()
	void RefreshServerList();

};
