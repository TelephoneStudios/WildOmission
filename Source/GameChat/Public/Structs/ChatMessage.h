// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ChatMessage.generated.h"

UENUM()
enum EChatMessageType
{
	STANDARD,
	CONNECTION_UPDATE,
	DEATH_NOTIFICTION
};

USTRUCT()
struct GAMECHAT_API FChatMessage
{
	GENERATED_BODY()

	FChatMessage();
	FChatMessage(
		const FString& InSenderName, const FString& InMessage,
		const bool& InSenderIsAdministrator, const double& InTimeRecieved,
		const TEnumAsByte<EChatMessageType>& InMessageType);

	UPROPERTY()
	FString SenderName;

	UPROPERTY()
	FString Message;

	UPROPERTY()
	bool SenderIsAdminisrator;

	UPROPERTY()
	double TimeRecieved;

	UPROPERTY()
	TEnumAsByte<EChatMessageType> MessageType;

};