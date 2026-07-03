 // Copyright Telephone Studios. All Rights Reserved.


#include "PlayWorldSelectionWidget.h"
#include "Components/Button.h"

UPlayWorldSelectionWidget::UPlayWorldSelectionWidget(const FObjectInitializer& ObjectInitializer) : UWorldSelectionWidget(ObjectInitializer)
{
	CreateNewWorldButton = nullptr;
	MultiplayerButton = nullptr;
}

void UPlayWorldSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CreateNewWorldButton->OnClicked.AddDynamic(this, &UPlayWorldSelectionWidget::BroadcastCreateNewWorldButtonClicked);
	MultiplayerButton->OnClicked.AddDynamic(this, &UPlayWorldSelectionWidget::BroadcastMultiplayerButtonClicked);
}

void UPlayWorldSelectionWidget::BroadcastCreateNewWorldButtonClicked()
{
	if (!OnCreateNewWorldButtonClicked.IsBound())
	{
		return;
	}

	OnCreateNewWorldButtonClicked.Broadcast();
}

void UPlayWorldSelectionWidget::BroadcastMultiplayerButtonClicked()
{
	if (!OnMultiplayerButtonClicked.IsBound())
	{
		return;
	}

	OnMultiplayerButtonClicked.Broadcast();
}