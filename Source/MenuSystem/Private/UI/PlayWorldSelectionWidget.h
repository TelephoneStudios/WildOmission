// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WorldSelectionWidget.h"
#include "PlayWorldSelectionWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCreateNewWorldButtonClickedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMultiplayerButtonClickedSignature);

UCLASS()
class UPlayWorldSelectionWidget : public UWorldSelectionWidget
{
	GENERATED_BODY()

public:
	UPlayWorldSelectionWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	FOnCreateNewWorldButtonClickedSignature OnCreateNewWorldButtonClicked;
	FOnMultiplayerButtonClickedSignature OnMultiplayerButtonClicked;
private:
	
	UPROPERTY(Meta = (BindWidget))
	class UButton* CreateNewWorldButton;

	UPROPERTY(Meta = (BindWidget))
	class UButton* MultiplayerButton;

	UFUNCTION()
	void BroadcastCreateNewWorldButtonClicked();

	UFUNCTION()
	void BroadcastMultiplayerButtonClicked();
};
