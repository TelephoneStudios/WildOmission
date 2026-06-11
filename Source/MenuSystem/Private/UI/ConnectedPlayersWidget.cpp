// Copyright Telephone Studios. All Rights Reserved.


#include "UI/ConnectedPlayersWidget.h"
#include "Components/Button.h"
#include "PlayerRowWidget.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"
#include "UObject/ConstructorHelpers.h"

UConnectedPlayersWidget::UConnectedPlayersWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	BackButton = nullptr;
	ConnectedPlayersPanel = nullptr;
	PlayerRowWidgetClass = nullptr;

	static ConstructorHelpers::FClassFinder<UPlayerRowWidget> PlayerRowWidgetBlueprint(TEXT("/Game/MenuSystem/UI/Players/WBP_PlayerRow"));
	if (PlayerRowWidgetBlueprint.Succeeded())
	{
		PlayerRowWidgetClass = PlayerRowWidgetBlueprint.Class;
	}
}

void UConnectedPlayersWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UConnectedPlayersWidget::Back);
	}
}

void UConnectedPlayersWidget::Refresh()
{
	ConnectedPlayersPanel->ClearChildren();

	// Get Game State
	AGameStateBase* GameState = GetWorld()->GetGameState();
	if (GameState == nullptr)
	{
		return;
	}

	// Get Each Player State
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (PlayerState == nullptr)
		{
			continue;
		}

		UPlayerRowWidget* PlayerRow = CreateWidget<UPlayerRowWidget>(this, PlayerRowWidgetClass);
		if (PlayerRow == nullptr)
		{
			continue;
		}

		// Create Row Per Player
		PlayerRow->Setup(PlayerState->GetPlayerName(), PlayerState->GetUniqueId()->ToString());
		PlayerRow->OnRequestRefresh.AddDynamic(this, &UConnectedPlayersWidget::Refresh);
		ConnectedPlayersPanel->AddChild(PlayerRow);
	}
}

void UConnectedPlayersWidget::Back()
{
	if (!OnBackButtonClicked.IsBound())
	{
		return;
	}

	OnBackButtonClicked.Broadcast();
}
