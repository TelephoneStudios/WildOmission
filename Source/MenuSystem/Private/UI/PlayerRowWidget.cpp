// Copyright Telephone Studios. All Rights Reserved.


#include "UI/PlayerRowWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "ServerAdministrators.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameSession.h"
#include "Interfaces/ServerAdministrator.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Log.h"

UPlayerRowWidget::UPlayerRowWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	NameTextBlock = nullptr;
	ViewProfileButton = nullptr;
	KickButton = nullptr;
	UniqueID = TEXT("");
}

void UPlayerRowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ViewProfileButton)
	{
		ViewProfileButton->OnClicked.AddDynamic(this, &UPlayerRowWidget::ViewProfile);
	}

	if (KickButton)
	{
		KickButton->OnClicked.AddDynamic(this, &UPlayerRowWidget::Kick);
	}
}

void UPlayerRowWidget::Setup(const FString& Name, const FString& InUniqueID)
{
	NameTextBlock->SetText(FText::FromString(Name));
	UniqueID = InUniqueID;

	APlayerState* OwningPlayerState = GetOwningPlayerState();
	APlayerController* OwningPlayerController = GetOwningPlayer();
	if (OwningPlayerState == nullptr || OwningPlayerController == nullptr)
	{
		return;
	}
	
	IServerAdministrator* ServerAdministrator = Cast<IServerAdministrator>(OwningPlayerController);
	if (ServerAdministrator == nullptr)
	{
		return;
	}

	if (OwningPlayerState->GetUniqueId()->ToString() == InUniqueID || !ServerAdministrator->IsAdministrator())
	{
		KickButton->SetIsEnabled(false);
		KickButton->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPlayerRowWidget::ViewProfile()
{
	const FString ProfileURL = FString::Printf(TEXT("https://steamcommunity.com/profiles/%s/"), *UniqueID);
	UKismetSystemLibrary::LaunchURL(ProfileURL);
}

void UPlayerRowWidget::Kick()
{
	IServerAdministrator* OwnerServerAdministrator = Cast<IServerAdministrator>(GetOwningPlayer());
	if (OwnerServerAdministrator == nullptr)
	{
		return;
	}

	OwnerServerAdministrator->KickPlayer(GetPlayerControllerForThis());

	if (OnRequestRefresh.IsBound())
	{
		OnRequestRefresh.Broadcast();
	}
}

APlayerController* UPlayerRowWidget::GetPlayerControllerForThis() const
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return nullptr;
	}

	AGameStateBase* GameState = World->GetGameState();
	if (GameState == nullptr)
	{
		return nullptr;
	}

	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (PlayerState == nullptr || PlayerState->GetUniqueId()->ToString() != UniqueID)
		{
			continue;
		}

		return PlayerState->GetPlayerController();
	}

	return nullptr;
}
