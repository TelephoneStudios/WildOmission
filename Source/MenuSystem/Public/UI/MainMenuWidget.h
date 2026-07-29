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
	void OpenPlayWorldSelectionMenu();
	UFUNCTION()
	void OpenWorldCreationMenu();
	UFUNCTION()
	void OpenWorldMenu();
	UFUNCTION()
	void OpenWorldMenuForWorld(const FString& WorldName);
	UFUNCTION()
	void HostGame(const FString& WorldName, const FString& ServerName, const bool IsMultiplayer, const bool IsFriendsOnly, const uint8& GameMode, const int32& MaxPlayerCount);
	UFUNCTION()
	void OpenRenameWorldMenu();
	UFUNCTION()
	void OpenDeleteWorldMenu();
	UFUNCTION()
	void OpenServerBrowserMenu();
	UFUNCTION()
	void OpenOptionsMenu();
	UFUNCTION()
	void OpenCharacterCustomizationMenu();
	UFUNCTION()
	void OpenWorkshopMenu();
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

	UFUNCTION(BlueprintCallable)
	bool CharacterCustomizationMenuOpen() const;

protected:
	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	class UWidgetSwitcher* MenuSwitcher;

	UPROPERTY(Meta = (BindWidget))
	class UButton* PlayButton;
	UPROPERTY(Meta = (BindWidget))
	class UButton* OptionsButton;
	UPROPERTY(Meta = (BindWidget))
	class UButton* CharacterCustomizationButton;
	UPROPERTY(Meta = (BindWidget))
	class UButton* WorkshopButton;
	UPROPERTY(Meta = (BindWidget))
	class UButton* FeedbackButton;
	UPROPERTY(Meta = (BindWidget))
	class UButton* CreditsButton;
	UPROPERTY(Meta = (BindWidget))
	class UButton* ExitButton;

	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* SplashTextBlock;

	UPROPERTY(Meta = (BindWidget))
	class UWidget* MainMenuPanel;
	UPROPERTY(Meta = (BindWidget))
	class UWidget* MainMenu;
	UPROPERTY(Meta = (BindWidget))
	class UWidget* UpdatingWorldsMenu;
	UPROPERTY(Meta = (BindWidget))
	class UWidget* WorldSelectionMenuPanel;
	UPROPERTY(Meta = (BindWidget))
	class UPlayWorldSelectionWidget* PlayWorldSelectionMenu;
	UPROPERTY(Meta = (BindWidget))
	class UWorldCreationWidget* WorldCreationMenu;
	UPROPERTY(Meta = (BindWidget))
	class UWidget* WorldMenuPanel;
	UPROPERTY(Meta = (BindWidget))
	class UWorldMenuWidget* WorldMenu;
	UPROPERTY(Meta = (BindWidget))
	class URenameWorldWidget* RenameWorldMenu;
	UPROPERTY(Meta = (BindWidget))
	class UDeleteWorldWidget* DeleteWorldMenu;
	UPROPERTY(Meta = (BindWidget))
	class UWidget* ServerBrowserMenuPanel;
	UPROPERTY(Meta = (BindWidget))
	class UServerBrowserWidget* ServerBrowserMenu;
	UPROPERTY(Meta = (BindWidget))
	class UWidget* OptionsMenuPanel;
	UPROPERTY(Meta = (BindWidget))
	class UOptionsWidget* OptionsMenu;
	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	class UCharacterCustomizationWidget* CharacterCustomizationMenu;
	UPROPERTY(Meta = (BindWidget))
	class UWorkshopMenuWidget* WorkshopMenu;
	UPROPERTY(Meta = (BindWidget))
	class UWidget* CreditsMenuPanel;
	UPROPERTY(Meta = (BindWidget))
	class UCreditsWidget* CreditsMenu;
	UPROPERTY(Meta = (BindWidget))
	class UErrorMessagePrompt* ErrorMessagePrompt;

	UPROPERTY(Meta = (BindWidget))
	class UWidget* InstallingWorkshopWorldsMenu;

private:
	

	class IMenuInterface* MenuInterface;
	
	UFUNCTION()
	void RefreshSplashText();

	UFUNCTION()
	void JoinServer(const uint32& ServerIndex);

	UFUNCTION()
	void RefreshServerList(bool IsDedicated);

};
