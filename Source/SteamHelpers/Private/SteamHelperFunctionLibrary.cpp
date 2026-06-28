// Copyright Telephone Studios. All Rights Reserved.


#include "SteamHelperFunctionLibrary.h"
#include "ThirdParty/Steamworks/sdk/public/steam/steam_api.h"
#include "Log.h"

static const int32 Game_App_ID = 2348700;

bool USteamHelperFunctionLibrary::IsDLCInstalled(int32 AppID)
{
	if (SteamUser() == nullptr)
	{
		return false;
	}

	return SteamApps()->BIsDlcInstalled(AppID);
}

bool USteamHelperFunctionLibrary::IsAppInstalled(int32 AppID)
{
	if (SteamUser() == nullptr)
	{
		return false;
	}

	return SteamApps()->BIsAppInstalled(AppID);
}

void USteamHelperFunctionLibrary::OpenStore(int32 DLC_AppID)
{
	if (SteamUser() == nullptr)
	{
		return;
	}

	SteamFriends()->ActivateGameOverlayToStore(DLC_AppID, k_EOverlayToStoreFlag_None);
}