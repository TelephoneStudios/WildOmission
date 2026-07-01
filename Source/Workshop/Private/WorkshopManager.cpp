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

void UWorkshopManager::UploadWorld(const FString& WorldName, const FString& WorkshopItemName, const FString& WorkshopItemDescription)
{
	if (SteamUGC() == nullptr)
	{
		UE_LOG(LogWorkshop, Warning, TEXT("Failed to upload workshop content, SteamUGC() returned nullptr"));
		return;
	}

	UploadInProgress = true;
	PendingUploadWorldName = WorldName;
	PendingUploadWorkshopItemName = WorkshopItemName;
	PendingUploadWorkshopItemDescription = WorkshopItemDescription;

	SteamAPICall_t SteamAPICall = SteamUGC()->CreateItem(SteamUtils()->GetAppID(), EWorkshopFileType::k_EWorkshopFileTypeCommunity);
	m_SteamCallResultCreateItem.Set(SteamAPICall, this, &UWorkshopManager::OnItemCreated);
}

bool UWorkshopManager::IsUploadInProgress() const
{
	return UploadInProgress;
}

EItemUpdateStatus UWorkshopManager::GetItemUploadStatus(float& OutPercent)
{
	if (SteamUGC() == nullptr)
	{
		//UE_LOG(LogWorkshop, Warning, TEXT("Failed to get item upload percentage, STEAM_UGC nullptr"));
		return EItemUpdateStatus::k_EItemUpdateStatusInvalid;
	}

	uint64_t BytesProcessed = 0;
	uint64_t BytesTotal = 1;
	EItemUpdateStatus Status = SteamUGC()->GetItemUpdateProgress(hUpdate, &BytesProcessed, &BytesTotal);

	OutPercent = 0.0f;
	if (Status != k_EItemUpdateStatusInvalid && BytesTotal > 0)
	{
		UE_LOG(LogWorkshop, Warning, TEXT("Upload percent bytes processed: %i, bytes total: %i, percent %f"), BytesProcessed, BytesTotal, OutPercent);
		OutPercent = (double)BytesProcessed / (double)BytesTotal;
	}

	return Status;
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

		UploadWorldContent(newFileID);
	
		UE_LOG(LogWorkshop, Display, TEXT("Finished updating item, submiting."));
	}
	else {
		// Handle error codes (e.g., k_EResultTimeout, k_EResultInsufficientPrivilege)
		UE_LOG(LogWorkshop, Warning, TEXT("Workshop item creation unspecified error"));
	}

}

void UWorkshopManager::WorkshopSubmittedCallback(SubmitItemUpdateResult_t* pCallback, bool bIOFailure)
{
	UE_LOG(LogWorkshop, Display, TEXT("Workshop item submitted"));
	UploadInProgress = false;
}

void UWorkshopManager::UploadWorldContent(PublishedFileId_t nFileID)
{
	hUpdate = SteamUGC()->StartItemUpdate(SteamUtils()->GetAppID(), nFileID);

	// Populate item metadata
	SteamUGC()->SetItemTitle(hUpdate, TCHAR_TO_ANSI(*PendingUploadWorkshopItemName));
	SteamUGC()->SetItemDescription(hUpdate, TCHAR_TO_ANSI(*PendingUploadWorkshopItemDescription));
	SteamUGC()->SetItemVisibility(hUpdate, ERemoteStoragePublishedFileVisibility::k_ERemoteStoragePublishedFileVisibilityPublic);
	
	// Content paths
	FString ContentPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) + TEXT("/SaveGames/") + PendingUploadWorldName;
	const FString IconPath = ContentPath + TEXT("/Icon.png");

	UE_LOG(LogWorkshop, Display, TEXT("Uploading world with content path: %s"), *ContentPath);
	UE_LOG(LogWorkshop, Display, TEXT("Uploading world with icon path: %s"), *IconPath);
	UE_LOG(LogWorkshop, Display, TEXT("Uploading world conent"));
	
	// Content
	SteamUGC()->SetItemContent(hUpdate, TCHAR_TO_ANSI(*ContentPath));
	SteamUGC()->SetItemPreview(hUpdate, TCHAR_TO_ANSI(*IconPath));

	SteamAPICall_t hSteamAPICall = SteamUGC()->SubmitItemUpdate(hUpdate, "Initial upload from client.");
	m_SteamCallSubmitItem.Set(hSteamAPICall, this, &UWorkshopManager::WorkshopSubmittedCallback);
}
