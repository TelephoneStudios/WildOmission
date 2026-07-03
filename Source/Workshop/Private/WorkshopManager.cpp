// Copyright Telephone Studios. All Rights Reserved.


#include "WorkshopManager.h"
#include "Containers/UnrealString.h"
#include "Engine/Texture2D.h"
#include "SaveManager.h"
#include "WorldInformation.h"
#include "Kismet/GameplayStatics.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Log.h"

static UWorkshopManager* WorkshopManagerInstance = nullptr;

void UWorkshopManager::OnCreation()
{
	WorkshopManagerInstance = this;
}

UWorkshopManager* UWorkshopManager::GetWorkshopManager()
{
	return WorkshopManagerInstance;
}

void UWorkshopManager::QueryPopularWorlds()
{
	if (!SteamUGC())
	{
		return;
	}

	// Create a query for all UGC types sorted by popularity
	UGCQueryHandle_t queryHandle = SteamUGC()->CreateQueryAllUGCRequest(
		k_EUGCQuery_RankedByVote,					// Sort order
		k_EUGCMatchingUGCType_Items_ReadyToUse,		// Matching type (regular workshop items)
		SteamUtils()->GetAppID(),					// App ID of the game the items belong to
		SteamUtils()->GetAppID(),					// App ID of the game creating the query
		1                                  // Page number (starts at 1)
	);

	if (queryHandle == k_UGCQueryHandleInvalid)
	{
		UE_LOG(LogWorkshop, Warning, TEXT("Failed to create UGC query handle."));
		return;
	}

	// Optional: Customize the query filters
	SteamUGC()->SetReturnLongDescription(queryHandle, true); // Include full text description
	SteamUGC()->SetReturnTotalOnly(queryHandle, false);     // Return actual items, not just counts

	// Send the query to Steam
	SteamAPICall_t apiCall = SteamUGC()->SendQueryUGCRequest(queryHandle);

	// Register the callback to handle the response asynchronously
	m_SteamCallResultQueryWorkshop.Set(apiCall, this, &UWorkshopManager::OnWorkshopQueryCompletedCallback);

	UE_LOG(LogWorkshop, Display, TEXT("Workshop query sent to steam..."));
}

void UWorkshopManager::SubscribeAndDownloadWorld(const FString& PublishedFileIdStr)
{
	if (!SteamUGC()) return;

	DownloadInProgress = true;

	// Convert string back to Steam's uint64 file ID layout
	//uint64 RawId = FString::ToHexBlob(*PublishedFileIdStr); // Or FCString::Atoll if using numeric mapping
	PublishedFileId_t FileId = FCString::Atoi64(*PublishedFileIdStr);
	DownloadFileId = FileId;

	// Step 1: Tell Steam to subscribe to the item
	SteamAPICall_t apiCall = SteamUGC()->SubscribeItem(FileId);
	m_CallResultSubscribe.Set(apiCall, this, &UWorkshopManager::OnSubscribeCompleted);
	m_CallbackItemInstalled.Set(apiCall, this, &UWorkshopManager::OnItemInstalled);
	m_CallbackDownloadResult.Set(apiCall, this, &UWorkshopManager::OnDownloadResult);
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

bool UWorkshopManager::IsDownloadInProgress() const
{
	return DownloadInProgress;
}

bool UWorkshopManager::IsWorkshopItemSubscribed(uint64 WorkshopItemID)
{
	// Ensure the Steam API is initialized and available
	if (SteamAPI_Init() && SteamUGC())
	{
		// Convert the raw uint64 ID into the Steamworks Type
		PublishedFileId_t TargetFileID = (PublishedFileId_t)WorkshopItemID;

		// Get the item state bitmask
		uint32 ItemState = SteamUGC()->GetItemState(TargetFileID);

		// Perform a bitwise AND check against the subscribed flag
		if (ItemState & k_EItemStateSubscribed)
		{
			return true; // The current user is actively subscribed
		}
	}

	return false; // Not initialized or not subscribed
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

float UWorkshopManager::GetItemDownloadProgress()
{
	if (SteamUGC() == nullptr)
	{
		return 0.0f;
	}

	uint64 BytesDownloaded = 0;
	uint64 BytesTotal = 0;
	uint32 ItemState = SteamUGC()->GetItemState(DownloadFileId);
	float ProgressFraction = 0.0f;

	if (ItemState & k_EItemStateDownloading)
	{
		// 2. Fetch the progress metrics
		bool bIsTracking = SteamUGC()->GetItemDownloadInfo(
			DownloadFileId,
			&BytesDownloaded,
			&BytesTotal
		);

		if (bIsTracking && BytesTotal > 0)
		{
			// 3. Calculate percentage (0.0f to 1.0f)
			ProgressFraction = (float)BytesDownloaded / (float)BytesTotal;

			UE_LOG(LogTemp, Log, TEXT("Mod %llu Download Progress: %.2f%% (%llu / %llu bytes)"),
				DownloadFileId, ProgressFraction, BytesDownloaded, BytesTotal);
		}
	}

	if (ItemState & k_EItemStateInstalled)
	{
		(void*)CopyWorldToSaveGamesFolder(DownloadFileId);

		// Add this to the transfer data, so it doesn't get processed twice
		FWorkshopTransferData TransferData;
		(void*)LoadTransferDataFromJsonFile(TransferData);
		TransferData.LastTransferCheck = FDateTime::UtcNow();
		FWorkshopDownload NewProcessedDownload;
		NewProcessedDownload.ItemID = DownloadFileId;
		NewProcessedDownload.FolderName = FString::FromInt(NewProcessedDownload.ItemID);
		TransferData.ProcessedDownloads.Add(NewProcessedDownload);
		(void*)SaveTransferDataToJsonFile(TransferData);

		DownloadInProgress = false;
	}

	return ProgressFraction;
}

void UWorkshopManager::OnWorkshopQueryCompletedCallback(SteamUGCQueryCompleted_t* pCallback, bool bIOFailure)
{
	TArray<FSteamWorkshopItemDetails> ParsedItems;
	// Check for network or internal errors
	if (bIOFailure || pCallback->m_eResult != k_EResultOK) 
	{
		AsyncTask(ENamedThreads::GameThread, [this, MovedItems = MoveTemp(ParsedItems)]()
			{
				if (OnWorkshopQueryCompleted.IsBound())
				{
					OnWorkshopQueryCompleted.Broadcast(false, MovedItems);
				}
			});

		UE_LOG(LogWorkshop, Error, TEXT("Couldn't fetch workshop items."));
		return;
	}

	UE_LOG(LogWorkshop, Display, TEXT("Successfully fetched %i items"), pCallback->m_unNumResultsReturned);

	// Loop through the returned results
	for (uint32 i = 0; i < pCallback->m_unNumResultsReturned; ++i) 
	{
		SteamUGCDetails_t details;
		if (SteamUGC()->GetQueryUGCResult(pCallback->m_handle, i, &details))
		{
			FSteamWorkshopItemDetails WorkshopItem;
			WorkshopItem.Title = FString(UTF8_TO_TCHAR(details.m_rgchTitle));
			WorkshopItem.Description = FString(UTF8_TO_TCHAR(details.m_rgchDescription));
			WorkshopItem.FileID = FString::Printf(TEXT("%llu"), details.m_nPublishedFileId);
			WorkshopItem.Likes = static_cast<int32>(details.m_unVotesUp);

			uint64 WorkshopItemID = details.m_nPublishedFileId;
			WorkshopItem.WorkshopURL = FString::Printf(TEXT("https://steamcommunity.com/%i"), WorkshopItemID);

			char URLBuffer[1024];
			if (SteamUGC()->GetQueryUGCPreviewURL(pCallback->m_handle, i, URLBuffer, sizeof(URLBuffer)))
			{
				WorkshopItem.PreviewURL = FString(UTF8_TO_TCHAR(URLBuffer));
			}
			
			ParsedItems.Add(WorkshopItem);

			UE_LOG(LogWorkshop, Display, TEXT("--- Item %i ---"), i + 1);
			UE_LOG(LogWorkshop, Display, TEXT("Title: %s"), *WorkshopItem.Title);
			UE_LOG(LogWorkshop, Display, TEXT("Likes: %i"), WorkshopItem.Likes);
		}
	}

	// Always release the query handle to prevent memory leaks
	SteamUGC()->ReleaseQueryUGCRequest(pCallback->m_handle);
	AsyncTask(ENamedThreads::GameThread, [this, MovedItems = MoveTemp(ParsedItems)]()
		{
			if (OnWorkshopQueryCompleted.IsBound())
			{
				OnWorkshopQueryCompleted.Broadcast(true, MovedItems);
			}
		});
	
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

void UWorkshopManager::OnSubscribeCompleted(RemoteStorageSubscribePublishedFileResult_t* pCallback, bool bIOFailure)
{
	if (bIOFailure || !pCallback || pCallback->m_eResult != k_EResultOK)
	{
		UE_LOG(LogTemp, Error, TEXT("Steam Workshop subscription failed."));
		return;
	}

	// Step 3: Explicitly force Steam to begin downloading the contents immediately
	// Second parameter 'true' forces high priority, bumping it to the top of Steam's download queue
	bool bDownloadStarted = SteamUGC()->DownloadItem(pCallback->m_nPublishedFileId, true);
	if (bDownloadStarted)
	{
		UE_LOG(LogTemp, Log, TEXT("Steam Workshop download initiated successfully."));
	}
}

void UWorkshopManager::OnDownloadResult(DownloadItemResult_t* pCallback, bool bIOFailure)
{
	if (!pCallback || pCallback->m_eResult != k_EResultOK) return;

	// Steam successfully finished the network download phase for an item.
	// It will automatically unpack it now; track completion via OnItemInstalled.}
}

void UWorkshopManager::OnItemInstalled(ItemInstalled_t* pCallback, bool bIOFailure)
{
	if (!pCallback) return;

	uint64 SizeOnDisk = 0;
	char FolderPath[4096]; // Buffer to hold system path string

	// Query Steam for where it placed the unpacked mod files
	bool bFoundPath = SteamUGC()->GetItemInstallInfo(
		pCallback->m_nPublishedFileId,
		&SizeOnDisk,
		FolderPath,
		UE_ARRAY_COUNT(FolderPath),
		nullptr
	);

	uint32 ItemState = SteamUGC()->GetItemState(pCallback->m_nPublishedFileId);

	FString ClearFileId = FString::Printf(TEXT("%llu"), pCallback->m_nPublishedFileId);
	FString AbsolutePath = FString(UTF8_TO_TCHAR(FolderPath));
	// this is never called for some reason
	// the lack of any documentation regarding this is giving me a headache
	if (ItemState & k_EItemStateInstalled)
	{
		DownloadInProgress = false;
	}
}

bool UWorkshopManager::CopyWorldToSaveGamesFolder(PublishedFileId_t FileId)
{
	if (SteamUGC() == nullptr)
	{
		return false;
	}

	// Copy the file
	uint64 SizeOnDisk = 0;
	char FolderPath[1024]; // Buffer array to store the system path
	uint32 FolderPathSize = sizeof(FolderPath);
	uint32 Timestamp = 0;
	bool IsInstalled = SteamUGC()->GetItemInstallInfo(FileId, &SizeOnDisk, FolderPath, FolderPathSize, &Timestamp);

	if (IsInstalled)
	{
		const FString SourcePath = FString(UTF8_TO_TCHAR(FolderPath));
		const FString FolderName = FPaths::GetCleanFilename(SourcePath);
		const FString DestinationPath = FPaths::ProjectSavedDir() + TEXT("SaveGames/") + FolderName;

		IFileManager& FileManager = IFileManager::Get();
		TArray<FString> FoundFiles;
		FileManager.FindFilesRecursive(FoundFiles, *SourcePath, TEXT("*.*"), true, false, false);

		if (FoundFiles.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Workshop directory is empty: %s"), *SourcePath);
			return false;
		}

		bool bAllFilesCopied = true;

		// 4. Loop through every discovered file and mirror it to the new destination
		for (const FString& SourceFilePath : FoundFiles)
		{
			// Get the relative path of the file compared to the original workshop root folder
			FString RelativePath = SourceFilePath;
			FPaths::MakePathRelativeTo(RelativePath, *(SourcePath + TEXT("/")));

			// Combine the relative path with our new SaveGames destination path
			FString DestinationFilePath = DestinationPath / RelativePath;

			// Perform the copy operation (true = overwrite existing files)
			uint32 CopyResult = FileManager.Copy(*DestinationFilePath, *SourceFilePath, true, false);

			if (CopyResult != COPY_OK)
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to copy file: %s -> %s"), *SourceFilePath, *DestinationFilePath);
				bAllFilesCopied = false;
			}
		}

		// Rename Folder to match world name
		UWorldInformation* DownloadedWorldInformation = Cast<UWorldInformation>(UGameplayStatics::CreateSaveGameObject(UWorldInformation::StaticClass()));
		FString SlotName = FolderName + TEXT("/WorldInformation");
		DownloadedWorldInformation = Cast<UWorldInformation>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
		FString WorldName = DownloadedWorldInformation->CreationInformation.Name;

		ASaveManager::RenameWorld(FolderName, WorldName);
	}

	return true;
}

bool UWorkshopManager::SaveTransferDataToJsonFile(const FWorkshopTransferData& Data)
{
	FString OutputString;
	if (!FJsonObjectConverter::UStructToJsonObjectString(Data, OutputString))
	{
		return false;
	}

	const FString Filename = FPaths::ProjectSavedDir() + TEXT("Workshop/WorkshopTransferData.json");

	return FFileHelper::SaveStringToFile(OutputString, *Filename, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}

bool UWorkshopManager::LoadTransferDataFromJsonFile(FWorkshopTransferData& OutTransferData)
{
	const FString Filename = FPaths::ProjectSavedDir() + TEXT("Workshop/WorkshopTransferData.json");
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *Filename))
	{
		return false;
	}

	return FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &OutTransferData, 0, 0);
}

void UWorkshopManager::CheckAndCopyNewWorkshopItems()
{
	if (SteamUGC() == nullptr)
	{
		return;
	}

	// Get all subscribed ids
	uint32 SubscribedCount = SteamUGC()->GetNumSubscribedItems();
	UE_LOG(LogWorkshop, Display, TEXT("Subscribed item count: %i"), SubscribedCount);
	if (SubscribedCount == 0)
	{
		return;
	}

	TArray<PublishedFileId_t> SubscribedItems;
	SubscribedItems.SetNum(SubscribedCount);
	SteamUGC()->GetSubscribedItems(SubscribedItems.GetData(), SubscribedCount);

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	FWorkshopTransferData TransferData;
	(void*)LoadTransferDataFromJsonFile(TransferData);

	// Scan through current workshop items
	for (uint32 i = 0; i < SubscribedCount; ++i)
	{
		PublishedFileId_t ItemID = SubscribedItems[i];
		uint32 ItemState = SteamUGC()->GetItemState(ItemID);
		
		// Could check for updates in the future but the world 
		// workshop will not support updates by its very nature
		if ((ItemState & k_EItemStateInstalled))
		{
			uint64 SizeOnDisk = 0;
			char PathBuffer[1024];
			uint32 SteamFolderTimestamp = 0;
			if (SteamUGC()->GetItemInstallInfo(ItemID, &SizeOnDisk, PathBuffer, sizeof(PathBuffer), &SteamFolderTimestamp))
			{
				FString SteamSourcePath = FString(UTF8_TO_TCHAR(PathBuffer));
				FString SteamWorkshopItemFolderName = FPaths::GetCleanFilename(SteamSourcePath);
				UE_LOG(LogWorkshop, Display, TEXT("Got subscribed workshop item with folder name: %s"), *SteamWorkshopItemFolderName);
				
				bool IsNew = true;
				if (!TransferData.ProcessedDownloads.IsEmpty())
				{
					for (const FWorkshopDownload& ProcessedDownload : TransferData.ProcessedDownloads)
					{
						if (ProcessedDownload.FolderName != SteamWorkshopItemFolderName)
						{
							continue;
						}
						// if folder of the same name has already been processed then this
						// isnt a new file
						IsNew = false;
					}
				}
			
				if (IsNew)
				{
					UE_LOG(LogWorkshop, Display, TEXT("New world detected: %s"), *SteamWorkshopItemFolderName);
					(void*)CopyWorldToSaveGamesFolder(ItemID);
					FWorkshopDownload NewProcessedDownload;
					NewProcessedDownload.FolderName = SteamWorkshopItemFolderName;
					NewProcessedDownload.ItemID = ItemID;
					TransferData.ProcessedDownloads.Add(NewProcessedDownload);
				}
				else
				{
					UE_LOG(LogWorkshop, Display, TEXT("World %s has already been processed"), *SteamWorkshopItemFolderName);
				}
			}
		}
	}
	TransferData.LastTransferCheck = FDateTime::UtcNow();
	(void*)SaveTransferDataToJsonFile(TransferData);
}
