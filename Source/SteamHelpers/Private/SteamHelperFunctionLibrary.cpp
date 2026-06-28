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

uint64 USteamHelperFunctionLibrary::UploadWorkshopItem()
{
	if (SteamUGC() == nullptr)
	{
		UE_LOG(LogSteamHelpers, Warning, TEXT("Failed to upload workshop content, SteamUGC() returned nullptr"));
		return 0;
	}

	SteamUGC()->CreateItem(Game_App_ID, EWorkshopFileType::k_EWorkshopFileTypeCommunity);

	PublishedFileId_t NewItemPublishedField = PublishedFileId_t();
	UGCUpdateHandle_t NewItemUpdateHandle = SteamUGC()->StartItemUpdate(Game_App_ID, NewItemPublishedField);
	const char* ItemTitle = "TestUpload";
	const char* ItemDescription = "Test item description.";
	const char* ItemFilePath = "Saved/SaveGames/New_World.sav";
	SteamUGC()->SetItemTitle(NewItemUpdateHandle, ItemTitle);
	SteamUGC()->SetItemDescription(NewItemUpdateHandle, ItemDescription);
	SteamUGC()->SetItemVisibility(NewItemUpdateHandle, ERemoteStoragePublishedFileVisibility::k_ERemoteStoragePublishedFileVisibilityPublic);
	SteamUGC()->SetItemContent(NewItemUpdateHandle, ItemFilePath);

	const char* ChangeNote = "Not applicable";
	SteamUGC()->SubmitItemUpdate(NewItemUpdateHandle, ChangeNote);

	return NewItemUpdateHandle;
}

bool USteamHelperFunctionLibrary::IsItemUpdateComplete(const int64& InUpdateHandle)
{
	if (SteamUGC() == nullptr)
	{
		UE_LOG(LogSteamHelpers, Warning, TEXT("Failed to query workshop content update status, SteamUGC() returned nullptr"));
		return false;
	}

	uint64* bytesProcessed = nullptr;
	uint64* bytesTotal = nullptr;
	SteamUGC()->GetItemUpdateProgress(InUpdateHandle, bytesProcessed, bytesTotal);

	return bytesProcessed == bytesTotal;
}
