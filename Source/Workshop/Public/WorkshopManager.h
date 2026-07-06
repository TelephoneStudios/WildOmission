// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#pragma warning(disable: 4996)
#include "ThirdParty/Steamworks/sdk/public/steam/steam_api.h"
#include "WorkshopManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorkshopItemSubmittedSignature);

USTRUCT(BlueprintType)
struct FWorkshopDownload
{
	GENERATED_BODY()

	UPROPERTY()
	uint64 ItemID;

	UPROPERTY()
	FString FolderName;

};

USTRUCT(BlueprintType)
struct FWorkshopTransferData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FWorkshopDownload> ProcessedDownloads;

	UPROPERTY()
	FDateTime LastTransferCheck;

};

USTRUCT(BlueprintType)
struct FSteamWorkshopItemDetails
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Steam Workshop")
	FString Title;

	UPROPERTY(BlueprintReadOnly, Category = "Steam Workshop")
	FString Description;

	UPROPERTY(BlueprintReadOnly, Category = "Steam Workshop")
	FString FileID;

	UPROPERTY(BlueprintReadOnly, Category = "Steam Workshop")
	int32 Likes = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Steam Workshop")
	FString PreviewURL;

	UPROPERTY()
	uint64 AuthorID = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Steam Workshop")
	UTexture2D* PreviewTexture = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Steam Workshop")
	FString WorkshopURL;

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWorkshopQueryCompletedSignature, bool, bSuccess, const TArray<FSteamWorkshopItemDetails>&, Items);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWorkshopItemReadySignature, const FString&, FileId, const FString&, AbsoluteDiskPath);

UCLASS()
class WORKSHOP_API UWorkshopManager : public UObject
{
	GENERATED_BODY()
public:
	
	void OnCreation();

	static UWorkshopManager* GetWorkshopManager();

	void CheckAndCopyNewWorkshopItems();

	void QueryPopularWorlds();
	void SubscribeAndDownloadWorld(const FString& PublishedFileIdStr);
	void UploadWorld(const FString& WorldName, const FString& WorkshopItemName, const FString& WorkshopItemDescription);
	
	bool IsUploadInProgress() const;
	bool IsDownloadInProgress() const;
	bool IsWorkshopItemSubscribed(uint64 WorkshopItemID);

	EItemUpdateStatus GetItemUploadStatus(float& OutPercent);
	float GetItemDownloadProgress();
	FOnWorkshopQueryCompletedSignature OnWorkshopQueryCompleted;
	FOnWorkshopItemSubmittedSignature OnWorkshopItemSubmitted;
	FOnWorkshopItemReadySignature OnWorkshopItemReady;

private:
	// Querying
	void OnWorkshopQueryCompletedCallback(SteamUGCQueryCompleted_t* pCallback, bool bIOFailure);
	// Uploading
	void OnItemCreated(struct CreateItemResult_t* pCallback, bool bIOFailure);
	void WorkshopSubmittedCallback(struct SubmitItemUpdateResult_t* pCallback, bool bIOFailure);
	void UploadWorldContent(PublishedFileId_t nFileID);
	// Downloading
	void OnSubscribeCompleted(RemoteStorageSubscribePublishedFileResult_t* pCallback, bool bIOFailure);
	void OnDownloadResult(DownloadItemResult_t* pCallback, bool bIOFailure);
	void OnItemInstalled(ItemInstalled_t* pCallback, bool bIOFailure);

	// returns if operation was successful
	bool CopyWorldToSaveGamesFolder(PublishedFileId_t FileId);

	bool SaveTransferDataToJsonFile(const FWorkshopTransferData& Data);
	bool LoadTransferDataFromJsonFile(FWorkshopTransferData& OutTransferData);

	bool UploadInProgress;
	bool DownloadInProgress;

	PublishedFileId_t DownloadFileId;
	UGCUpdateHandle_t hUpdate;

	// Steam call result wrappers
	CCallResult<UWorkshopManager, SteamUGCQueryCompleted_t> m_SteamCallResultQueryWorkshop;
	CCallResult<UWorkshopManager, CreateItemResult_t> m_SteamCallResultCreateItem;
	CCallResult<UWorkshopManager, SubmitItemUpdateResult_t> m_SteamCallSubmitItem;
	CCallResult<UWorkshopManager, RemoteStorageSubscribePublishedFileResult_t> m_CallResultSubscribe;
	CCallResult<UWorkshopManager, ItemInstalled_t> m_CallbackItemInstalled;
	CCallResult<UWorkshopManager, DownloadItemResult_t> m_CallbackDownloadResult;

	FString PendingUploadWorldName;
	FString PendingUploadWorkshopItemName;
	FString PendingUploadWorkshopItemDescription;

};
