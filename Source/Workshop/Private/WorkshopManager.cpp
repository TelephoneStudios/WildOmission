// Copyright Telephone Studios. All Rights Reserved.


#include "WorkshopManager.h"
#include "Log.h"

static const int32 Game_App_ID = 2348700;
static UWorkshopManager* Instance = nullptr;

void UWorkshopManager::OnCreation()
{
	Instance = this;
}

UWorkshopManager* UWorkshopManager::GetWorkshopManager()
{
	return Instance;
}

void UWorkshopManager::UploadWorkshopItem()
{
	if (SteamUGC() == nullptr)
	{
		UE_LOG(LogWorkshop, Warning, TEXT("Failed to upload workshop content, SteamUGC() returned nullptr"));
		return;
	}

	SteamAPICall_t SteamAPICall = SteamUGC()->CreateItem(Game_App_ID, EWorkshopFileType::k_EWorkshopFileTypeCommunity);
	m_SteamCallResultCreateItem.Set(SteamAPICall, this, &UWorkshopManager::OnItemCreated);
}

bool UWorkshopManager::IsItemUpdateComplete(const int64& InUpdateHandle)
{
	if (SteamUGC() == nullptr)
	{
		UE_LOG(LogWorkshop, Warning, TEXT("Failed to query workshop content update status, SteamUGC() returned nullptr"));
		return false;
	}

	uint64* bytesProcessed = nullptr;
	uint64* bytesTotal = nullptr;
	SteamUGC()->GetItemUpdateProgress(InUpdateHandle, bytesProcessed, bytesTotal);

	return bytesProcessed == bytesTotal;
}

void UWorkshopManager::OnItemCreated(CreateItemResult_t* pCallback, bool bIOFailure)
{

	UE_LOG(LogWorkshop, Display, TEXT("OnItemCreated callback"));
	// 1. Check for engine/network failure
	if (bIOFailure) {
		UE_LOG(LogWorkshop, Warning, TEXT("Workshop item creation failure"));
		return;
	}

	// 2. Check if Steam successfully created the file backend
	if (pCallback->m_eResult == k_EResultOK) {

		// SUCCESS: Here is your PublishedFileId_t
		PublishedFileId_t newFileID = pCallback->m_nPublishedFileId;

		// Store this ID to run updates (Title, Description, File uploads, etc.)
		//this->m_SavedPublishedFileID = newFileID;

		// 3. Check if the user needs to accept the Steam Workshop Legal Agreement
		if (pCallback->m_bUserNeedsToAcceptWorkshopLegalAgreement) {
			SteamFriends()->ActivateGameOverlayToWebPage("steam://url/CommunityFilePage/");
		}

		UGCUpdateHandle_t NewItemUpdateHandle = SteamUGC()->StartItemUpdate(Game_App_ID, newFileID);
		/*const char* ItemTitle = "TestUpload";
		const char* ItemDescription = "Test item description.";
		const char* ItemFilePath = "Saved/SaveGames/New_World.sav";*/

		SteamUGC()->SetItemTitle(NewItemUpdateHandle, "Test Upload");
		SteamUGC()->SetItemDescription(NewItemUpdateHandle, "Test Item Description");
		SteamUGC()->SetItemVisibility(NewItemUpdateHandle, ERemoteStoragePublishedFileVisibility::k_ERemoteStoragePublishedFileVisibilityPublic);

		SteamUGC()->SetItemContent(NewItemUpdateHandle, "C:/Users/Larch/Documents/GitHub/WildOmission/Saved/SaveGames/");
		SteamUGC()->SetItemPreview(NewItemUpdateHandle, "C:/Users/Larch/Documents/GitHub/WildOmission/Saved/AutoScreenshot.png");

		SteamUGC()->SubmitItemUpdate(NewItemUpdateHandle, "Initial Upload");

		UE_LOG(LogWorkshop, Display, TEXT("OnItemCreated success and Updates submitted"));
	}
	else {
		// Handle error codes (e.g., k_EResultTimeout, k_EResultInsufficientPrivilege)
	}

}
