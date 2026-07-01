// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#pragma warning(disable: 4996)
#include "ThirdParty/Steamworks/sdk/public/steam/steam_api.h"
#include "WorkshopManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorkshopItemSubmittedSignature);

USTRUCT(BlueprintType)
struct FSteamWorkshopItemDetails
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Steam Workshop")
	FString Title;

	UPROPERTY(BlueprintReadOnly, Category = "Steam Workshop")
	FString FileID;

	UPROPERTY(BlueprintReadOnly, Category = "Steam Workshop")
	int32 Likes = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWorkshopQueryCompletedSignature, bool, bSuccess, const TArray<FSteamWorkshopItemDetails>&, Items);

UCLASS()
class WORKSHOP_API UWorkshopManager : public UObject
{
	GENERATED_BODY()
public:
	
	void OnCreation();

	static UWorkshopManager* GetWorkshopManager();

	void QueryPopularWorlds();
	void UploadWorld(const FString& WorldName, const FString& WorkshopItemName, const FString& WorkshopItemDescription);
	
	bool IsUploadInProgress() const;
	EItemUpdateStatus GetItemUploadStatus(float& OutPercent);
	FOnWorkshopQueryCompletedSignature OnWorkshopQueryCompleted;
	FOnWorkshopItemSubmittedSignature OnWorkshopItemSubmitted;

private:
	// Querying
	void OnWorkshopQueryCompletedCallback(SteamUGCQueryCompleted_t* pCallback, bool bIOFailure);
	// Uploading
	void OnItemCreated(struct CreateItemResult_t* pCallback, bool bIOFailure);
	void WorkshopSubmittedCallback(struct SubmitItemUpdateResult_t* pCallback, bool bIOFailure);
	void UploadWorldContent(PublishedFileId_t nFileID);

	bool UploadInProgress;
	UGCUpdateHandle_t hUpdate;

	// Steam call result wrappers
	CCallResult<UWorkshopManager, SteamUGCQueryCompleted_t> m_SteamCallResultQueryWorkshop;
	CCallResult<UWorkshopManager, CreateItemResult_t> m_SteamCallResultCreateItem;
	CCallResult<UWorkshopManager, SubmitItemUpdateResult_t> m_SteamCallSubmitItem;

	FString PendingUploadWorldName;
	FString PendingUploadWorkshopItemName;
	FString PendingUploadWorkshopItemDescription;

};
