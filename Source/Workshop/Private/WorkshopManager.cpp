// Copyright Telephone Studios. All Rights Reserved.


#include "WorkshopManager.h"
#include "Containers/UnrealString.h"
#include "HttpModule.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Engine/Texture2D.h"
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

	// Convert string back to Steam's uint64 file ID layout
	//uint64 RawId = FString::ToHexBlob(*PublishedFileIdStr); // Or FCString::Atoll if using numeric mapping
	PublishedFileId_t FileId = FCString::Atoi64(*PublishedFileIdStr);

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
			this->CachedWorkshopItems = MovedItems;

			for (int32 Index = 0; Index < this->CachedWorkshopItems.Num(); ++Index)
			{
				// TODO Move to workshop item UI
				if (!this->CachedWorkshopItems[Index].PreviewURL.IsEmpty())
				{
					DownloadPreviewTexture(this->CachedWorkshopItems[Index].PreviewURL, Index);
				}
			}

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

	if (bFoundPath)
	{
		FString ClearFileId = FString::Printf(TEXT("%llu"), pCallback->m_nPublishedFileId);
		FString AbsolutePath = FString(UTF8_TO_TCHAR(FolderPath));

		// Bounce back to Unreal's Main GameThread before executing delegates
		AsyncTask(ENamedThreads::GameThread, [this, ClearFileId, AbsolutePath]()
			{
				// Mod files are ready for Unreal to mount, load Pax files, or read configs!
				OnWorkshopItemReady.Broadcast(ClearFileId, AbsolutePath);
			});
	}
}

void UWorkshopManager::DownloadPreviewTexture(const FString& URL, const int32& ItemIndex)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(URL);
	Request->SetVerb(TEXT("GET"));

	// Use a lambda binder instead of BindUObject to cleanly pass the integer index through the HTTP event
	Request->OnProcessRequestComplete().BindLambda([this, ItemIndex](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			if (!bWasSuccessful || !Response.IsValid() || Response->GetContentLength() <= 0) return;

			// 1. Get the raw binary payload array
			const TArray<uint8>& RawImageData = Response->GetContent();

			// 2. Detect image format automatically (PNG, JPEG, etc.) via ImageWrapper
			IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
			EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(RawImageData.GetData(), RawImageData.Num());

			if (ImageFormat == EImageFormat::Invalid) return;

			TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
			if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(RawImageData.GetData(), RawImageData.Num())) return;

			// 3. Decompress the image into uncompressed raw RGBA byte data
			TArray<uint8> UncompressedBGRA;
			if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA)) return;

			// 4. Construct a new Transient Texture on the GameThread safely
			int32 Width = ImageWrapper->GetWidth();
			int32 Height = ImageWrapper->GetHeight();

			UTexture2D* LoadedTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
			if (!LoadedTexture) return;

			// 5. Bulk copy the decoded pixel array directly into the texture's platform memory
			void* TextureData = LoadedTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(TextureData, UncompressedBGRA.GetData(), UncompressedBGRA.Num());
			LoadedTexture->GetPlatformData()->Mips[0].BulkData.Unlock();

			// Update the texture properties so the GPU renders it correctly
			LoadedTexture->UpdateResource();

			// 5. Inject the texture back into the cached array matching this index
			if (this->CachedWorkshopItems.IsValidIndex(ItemIndex))
			{
				this->CachedWorkshopItems[ItemIndex].PreviewTexture = LoadedTexture;

				// 6. Broadcast a separate "Item Image Updated" delegate so the UI refresh button updates just this specific slot!
				//OnItemImageUpdated.Broadcast(ItemIndex, LoadedTexture);
			}
		});

	Request->ProcessRequest();
}

void UWorkshopManager::OnPreviewDownloaded(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid() || Response->GetContentLength() <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to download workshop preview image."));
		return;
	}

	// 1. Get the raw binary payload array
	const TArray<uint8>& RawImageData = Response->GetContent();

	// 2. Detect image format automatically (PNG, JPEG, etc.) via ImageWrapper
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(RawImageData.GetData(), RawImageData.Num());

	if (ImageFormat == EImageFormat::Invalid) return;

	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
	if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(RawImageData.GetData(), RawImageData.Num())) return;

	// 3. Decompress the image into uncompressed raw RGBA byte data
	TArray<uint8> UncompressedBGRA;
	if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA)) return;

	// 4. Construct a new Transient Texture on the GameThread safely
	int32 Width = ImageWrapper->GetWidth();
	int32 Height = ImageWrapper->GetHeight();

	UTexture2D* LoadedTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	if (!LoadedTexture) return;

	// 5. Bulk copy the decoded pixel array directly into the texture's platform memory
	void* TextureData = LoadedTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, UncompressedBGRA.GetData(), UncompressedBGRA.Num());
	LoadedTexture->GetPlatformData()->Mips[0].BulkData.Unlock();

	// Update the texture properties so the GPU renders it correctly
	LoadedTexture->UpdateResource();

	// 6. Broadcast your texture out to UI or Materials!
	// OnPreviewTextureReady.Broadcast(LoadedTexture);
}