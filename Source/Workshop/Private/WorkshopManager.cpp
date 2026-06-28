// Copyright Telephone Studios. All Rights Reserved.


#include "WorkshopManager.h"
#include "Log.h"

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

	SteamAPICall_t SteamAPICall = SteamUGC()->CreateItem(SteamUtils()->GetAppID(), EWorkshopFileType::k_EWorkshopFileTypeCommunity);
	m_SteamCallResultCreateItem.Set(SteamAPICall, this, &UWorkshopManager::OnItemCreated);
}

float UWorkshopManager::GetItemUploadPercentage() const
{
	if (SteamUGC() == nullptr)
	{
		UE_LOG(LogWorkshop, Warning, TEXT("Failed to get item upload percentage, STEAM_UGC nullptr"));
		return 0.0f;
	}

	uint64_t BytesProcessed = 0;
	uint64_t BytesTotal = 0;
	SteamUGC()->GetItemUpdateProgress(UploadHandle, &BytesProcessed, &BytesTotal);

	return static_cast<float>(BytesProcessed) / static_cast<float>(BytesTotal);
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
	return false;
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

		UploadHandle = SteamUGC()->StartItemUpdate(SteamUtils()->GetAppID(), newFileID);
		/*const char* ItemTitle = "TestUpload";
		const char* ItemDescription = "Test item description.";
		const char* ItemFilePath = "Saved/SaveGames/New_World.sav";*/

		SteamUGC()->SetItemTitle(UploadHandle, "Test Upload");
		SteamUGC()->SetItemDescription(UploadHandle, "Test Item Description");
		SteamUGC()->SetItemVisibility(UploadHandle, ERemoteStoragePublishedFileVisibility::k_ERemoteStoragePublishedFileVisibilityPublic);

		SteamUGC()->SetItemContent(UploadHandle, "C:/Users/Larch/Documents/GitHub/WildOmission/Saved/SaveGames/");
		SteamUGC()->SetItemPreview(UploadHandle, "C:/Users/Larch/Documents/GitHub/WildOmission/Saved/AutoScreenshot.png");

		SteamAPICall_t SubmitAPICall = SteamUGC()->SubmitItemUpdate(UploadHandle, "Initial Upload");
		m_SteamCallSubmitItem.Set(SubmitAPICall, this, &UWorkshopManager::WorkshopSubmittedCallback);
		
		UE_LOG(LogWorkshop, Display, TEXT("Finished updating item, submiting."));
	}
	else {
		// Handle error codes (e.g., k_EResultTimeout, k_EResultInsufficientPrivilege)
	}

}

void UWorkshopManager::WorkshopSubmittedCallback(SubmitItemUpdateResult_t* pCallback, bool bIOFailure)
{
	if (!IsValid(this))
	{
		return;
	}

	if (OnWorkshopItemSubmitted.IsBound())
	{
		OnWorkshopItemSubmitted.Broadcast();
	}
}
