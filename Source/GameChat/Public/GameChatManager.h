// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Structs/ChatMessage.h"
#include "GameChatManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChatMessageRecievedSignature);

UCLASS()
class GAMECHAT_API AGameChatManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGameChatManager();
	
	void SendMessage(APlayerState* SenderPlayerState, const FString& Message, const TEnumAsByte<EChatMessageType>& MessageType = EChatMessageType::STANDARD);

	static void SetGameChatManager(AGameChatManager* NewInstance);
	static AGameChatManager* GetGameChatManager();

	static bool IsValidMessage(const FChatMessage& ChatMessage);
	
	TArray<FChatMessage> GetChatMessages();

	FOnChatMessageRecievedSignature OnMessageRecieved;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(VisibleAnywhere)
	TArray<FChatMessage> ChatMessages;

	UFUNCTION(NetMulticast, Reliable)
	void Multi_PushMessageToClients(const FChatMessage& ChatMessage);

};
