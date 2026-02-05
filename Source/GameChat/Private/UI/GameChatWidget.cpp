// Copyright Telephone Studios. All Rights Reserved.


#include "UI/GameChatWidget.h"
#include "ChatMessageWidget.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "Interfaces/PlayerControllerMessageSender.h"
#include "GameFramework/PlayerState.h"
#include "GameChatManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Structs/ChatMessage.h"
#include "Log.h"

UGameChatWidget::UGameChatWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	MessageContainerPanel = nullptr;
	MessagePanel = nullptr;
	MessageBox = nullptr;
	
	ChatMessageClass = nullptr;

	static ConstructorHelpers::FClassFinder<UChatMessageWidget> ChatMessageBlueprint(TEXT("/Game/GameChat/UI/WBP_ChatMessage"));
	if (ChatMessageBlueprint.Succeeded())
	{
		ChatMessageClass = ChatMessageBlueprint.Class;
	}

	Opened = false;
	HideUnlessOpen = false;
}

void UGameChatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	MessageContainerPanel->ClearChildren();	
	Close();
	MessageBox->OnTextCommitted.AddDynamic(this, &UGameChatWidget::OnMessageBoxTextCommitted);
	AGameChatManager* ChatManager = AGameChatManager::GetGameChatManager();
	if (ChatManager == nullptr)
	{
		UE_LOG(LogGameChat, Warning, TEXT("Couldn't retrieve game chat handler."));
		return;
	}

	ChatManager->OnMessageRecieved.AddDynamic(this, &UGameChatWidget::RefreshMessages);
	RefreshMessages();
}

void UGameChatWidget::RefreshMessages()
{
	AGameChatManager* ChatManager = AGameChatManager::GetGameChatManager();
	APlayerState* OwnerPlayerState = GetOwningPlayerState();
	if (ChatManager == nullptr || OwnerPlayerState == nullptr)
	{
		return;
	}
	
	MessageContainerPanel->ClearChildren();
	const FString OurPlayerNetName = OwnerPlayerState->GetPlayerName();

	TArray<FChatMessage> Messages;
	Messages = ChatManager->GetChatMessages();

	if (Messages.IsEmpty())
	{
		return;
	}

	for (int32 i = Messages.Num()-1; i > -1; --i)
	{
		const FChatMessage& Message = Messages[i];

		if (Message.MessageType == EChatMessageType::CONNECTION_UPDATE && Message.SenderName == OurPlayerNetName)
		{
			continue;
		}

		UChatMessageWidget* MessageWidget = CreateWidget<UChatMessageWidget>(this, ChatMessageClass);
		if (MessageWidget == nullptr)
		{
			continue;
		}

		MessageWidget->Setup(this, Message);

		if (HideUnlessOpen)
		{
			MessageWidget->SetVisibility(ESlateVisibility::Hidden);
		}
		MessageContainerPanel->AddChild(MessageWidget);
	}

	MessageContainerPanel->ScrollToEnd();
}

void UGameChatWidget::Open()
{
	Opened = true;
	MessagePanel->SetVisibility(ESlateVisibility::Visible);
	MessageContainerPanel->SetVisibility(ESlateVisibility::Visible);
	MessageBox->SetFocus();
	
	// Show all old messages
	for (UWidget* Widget : MessageContainerPanel->GetAllChildren())
	{
		Widget->SetVisibility(ESlateVisibility::Visible);
	}

	MessageContainerPanel->ScrollToEnd();
}

void UGameChatWidget::Close()
{
	Opened = false;
	MessagePanel->SetVisibility(ESlateVisibility::Collapsed);

	if (HideUnlessOpen)
	{
		MessageContainerPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

bool UGameChatWidget::IsOpen() const
{
	return Opened;
}

void UGameChatWidget::SetHideUnlessOpen(bool InHideUnlessOpen)
{
	HideUnlessOpen = InHideUnlessOpen;
}

bool UGameChatWidget::GetHideUnlessOpen() const
{
	return HideUnlessOpen;
}

void UGameChatWidget::OnMessageBoxTextCommitted(const FText& MessageBoxText, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::Type::OnEnter)
	{
		AttemptSendMessage();
	}
	else if (CommitMethod != ETextCommit::Type::OnEnter && OnToggleRequested.IsBound())
	{
		OnToggleRequested.Broadcast();
	}
}

void UGameChatWidget::AttemptSendMessage()
{
	const bool MessageBoxEmpty = MessageBox->GetText().ToString().Len() == 0;

	IPlayerControllerMessageSender* MessageSender = Cast<IPlayerControllerMessageSender>(GetOwningPlayer());
	APlayerState* OwningPlayerState = GetOwningPlayerState();
	
	// Return if no message was typed
	if (MessageBoxEmpty || MessageSender == nullptr || OwningPlayerState == nullptr)
	{
		UE_LOG(LogGameChat, Display, TEXT("Cannot send nothing in game chat."));
		return;
	}

	MessageSender->SendMessage(OwningPlayerState, MessageBox->GetText().ToString());
	
	MessageBox->SetText(FText());
}