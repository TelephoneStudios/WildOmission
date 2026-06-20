// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SteamHelperFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class STEAMHELPERS_API USteamHelperFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable, META = (DisplayName = "Has DLC Installed", CompactNodeTitle = "Has DLC Installed", Category = "Steam Helper Functions"))
	static bool IsDLCInstalled(int32 AppID);

	UFUNCTION(BlueprintCallable, META = (DisplayName = "Game Installed", CompactNodeTitle = "Game Installed", Category = "Steam Helper Functions"))
	static bool IsAppInstalled(int32 AppID);

	UFUNCTION(BlueprintCallable, META = (DisplayName = "Open Store", CompactNodeTitle = "Open Store", Category = "Steam Helper Functions"))
	static void OpenStore(int32 DLC_AppID);
};
