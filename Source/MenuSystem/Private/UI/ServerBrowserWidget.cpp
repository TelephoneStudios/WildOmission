// Copyright Telephone Studios. All Rights Reserved.


#include "ServerBrowserWidget.h"
#include "ServerRowWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Color/UIColors.h"

UServerBrowserWidget::UServerBrowserWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	//DedicatedServersButton = nullptr;
	PlayerHostedWorldsButton = nullptr;
	ServerList = nullptr;
	JoinButton = nullptr;
	CancelButton = nullptr;
	RefreshListButton = nullptr;
	RefreshListButtonText = nullptr;
	ServerRowWidgetClass = nullptr;

	// support for dedicated servers has been depricated but may come back
	// this used to be true by default
	IsDedicatedList = false;

	static ConstructorHelpers::FClassFinder<UServerRowWidget> ServerRowWidgetBPClass(TEXT("/Game/MenuSystem/UI/Server/WBP_ServerRow"));
	if (ServerRowWidgetBPClass.Succeeded())
	{
		ServerRowWidgetClass = ServerRowWidgetBPClass.Class;
	}

	static ConstructorHelpers::FClassFinder<UTextBlock> TextBlockBlueprint(TEXT("/Game/CustomUI/WBP_UI_Text_Normal"));
	if (TextBlockBlueprint.Succeeded())
	{
		TextBlockClass = TextBlockBlueprint.Class;
	}
}

void UServerBrowserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//SwitchToDedicatedList();
	SwitchToPlayerHostedWorldsList();

	//DedicatedServersButton->OnClicked.AddDynamic(this, &UServerBrowserWidget::SwitchToDedicatedList);
	PlayerHostedWorldsButton->OnClicked.AddDynamic(this, &UServerBrowserWidget::SwitchToPlayerHostedWorldsList);

	JoinButton->SetIsEnabled(false);

	JoinButton->OnClicked.AddDynamic(this, &UServerBrowserWidget::BroadcastJoinButtonClicked);
	RefreshListButton->OnClicked.AddDynamic(this, &UServerBrowserWidget::BroadcastRefreshButtonClicked);
	CancelButton->OnClicked.AddDynamic(this, &UServerBrowserWidget::BroadcastCancelButtonClicked);
}

void UServerBrowserWidget::SetServerList(TArray<FServerData> ServerNames)
{
	UWorld* World = GetWorld();

	if (World == nullptr)
	{
		return;
	}

	ServerList->ClearChildren();

	uint32 i = 0;
	for (const FServerData& ServerData : ServerNames)
	{
		UServerRowWidget* Row = CreateWidget<UServerRowWidget>(World, ServerRowWidgetClass);
		if (Row == nullptr)
		{
			return;
		}
		Row->Setup(i, ServerData);
		Row->OnSelected.AddDynamic(this, &UServerBrowserWidget::SelectServerIndex);
		++i;

		ServerList->AddChild(Row);
	}

	FString RefreshString = TEXT("Refresh");
	RefreshListButtonText->SetText(FText::FromString(RefreshString));
}

void UServerBrowserWidget::SwitchToDedicatedList()
{
	IsDedicatedList = true;

	FUIColor* SelectedColor = UUIColors::GetBaseColor(TEXT("Blue"));
	FUIColor* UnselectedColor = UUIColors::GetBaseColor(TEXT("Gray"));
	if (SelectedColor == nullptr || UnselectedColor == nullptr)
	{
		return;
	}

	//DedicatedServersButton->SetBackgroundColor(SelectedColor->Default);
	PlayerHostedWorldsButton->SetBackgroundColor(UnselectedColor->Default);

	BroadcastRefreshButtonClicked();
}

void UServerBrowserWidget::SwitchToPlayerHostedWorldsList()
{
	IsDedicatedList = false;

	FUIColor* SelectedColor = UUIColors::GetBaseColor(TEXT("Blue"));
	FUIColor* UnselectedColor = UUIColors::GetBaseColor(TEXT("Gray"));
	if (SelectedColor == nullptr || UnselectedColor == nullptr)
	{
		return;
	}

	//DedicatedServersButton->SetBackgroundColor(UnselectedColor->Default);
	PlayerHostedWorldsButton->SetBackgroundColor(SelectedColor->Default);

	BroadcastRefreshButtonClicked();
}

void UServerBrowserWidget::UpdateServerListChildren()
{
	for (int32 i = 0; i < ServerList->GetChildrenCount(); ++i)
	{
		UServerRowWidget* Row = Cast<UServerRowWidget>(ServerList->GetChildAt(i));
		if (Row == nullptr)
		{
			return;
		}
		Row->SetSelected(SelectedServerIndex.IsSet() && SelectedServerIndex.GetValue() == i);
	}
}

void UServerBrowserWidget::SelectServerIndex(const uint32& SelectedIndex)
{
	SelectedServerIndex = SelectedIndex;
	JoinButton->SetIsEnabled(true);
	UpdateServerListChildren();
}

void UServerBrowserWidget::BroadcastRefreshButtonClicked()
{
	if (TextBlockClass && WidgetTree)
	{
		UTextBlock* SearchingTextBlock = WidgetTree->ConstructWidget<UTextBlock>(TextBlockClass);//NewObject<UTextBlock>(this, TextBlockClass->StaticClass());
		//SearchingTextBlock->Register
		if (SearchingTextBlock)
		{
			ServerList->ClearChildren();
			SearchingTextBlock->SetText(FText::FromString(TEXT("Searching for servers, please wait...")));
			ServerList->AddChild(SearchingTextBlock);
		}
	}
	
	const FString WaitingString = TEXT("...");
	RefreshListButtonText->SetText(FText::FromString(WaitingString));

	SelectedServerIndex.Reset();
	JoinButton->SetIsEnabled(false);

	if (!OnRefreshButtonClicked.IsBound())
	{
		return;
	}

	// pass IsDedicatedList in for 
	OnRefreshButtonClicked.Broadcast(IsDedicatedList);
}

void UServerBrowserWidget::OnOpen()
{
	BroadcastRefreshButtonClicked();
}

void UServerBrowserWidget::BroadcastJoinButtonClicked()
{
	if (!OnJoinButtonClicked.IsBound() || !SelectedServerIndex.IsSet())
	{
		return;
	}

	OnJoinButtonClicked.Broadcast(SelectedServerIndex.GetValue());
}

void UServerBrowserWidget::BroadcastCancelButtonClicked()
{
	if (!OnCancelButtonClicked.IsBound())
	{
		return;
	}

	OnCancelButtonClicked.Broadcast();
}
