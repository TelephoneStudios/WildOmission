// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MenuInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMenuInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MENUSYSTEM_API IMenuInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// TODO make separate functions for server and lobby

	UFUNCTION()
	virtual void StartSingleplayer(const FString& WorldName, const uint8& GameMode) = 0;

	UFUNCTION()
	virtual void HostServer(const FString& ServerName, const FString& WorldName, bool FriendsOnly = false, const int32& MaxPlayerCount = 8, const uint8& GameMode = 0) = 0;
	
	UFUNCTION()
	virtual void JoinServer(const uint32& Index) = 0;

	UFUNCTION()
	virtual void RefreshServerList(bool IsDedicated) = 0;

	UFUNCTION()
	virtual void QuitToMenu() = 0;

};
