// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#pragma warning(disable: 4996)
#include "ThirdParty/Steamworks/sdk/public/steam/steam_api.h"
#include "WorkshopManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorkshopItemSubmittedSignature);

/**
 * 
 */
UCLASS()
class WORKSHOP_API UWorkshopManager : public UObject
{
	GENERATED_BODY()
public:
	
	void OnCreation();

	static UWorkshopManager* GetWorkshopManager();

	void UploadWorld(const FString& WorldName, const FString& WorkshopItemName, const FString& WorkshopItemDescription);
	
	bool IsUploadInProgress() const;
	EItemUpdateStatus GetItemUploadStatus(float& OutPercent);

	FOnWorkshopItemSubmittedSignature OnWorkshopItemSubmitted;

private:
	// Handler for the asynchronous call result
	void OnItemCreated(struct CreateItemResult_t* pCallback, bool bIOFailure);
	void WorkshopSubmittedCallback(struct SubmitItemUpdateResult_t* pCallback, bool bIOFailure);
	void UploadWorldContent(PublishedFileId_t nFileID);

	bool UploadInProgress;
	UGCUpdateHandle_t hUpdate;

	// Steam call result wrapper
	CCallResult<UWorkshopManager, CreateItemResult_t> m_SteamCallResultCreateItem;
	CCallResult<UWorkshopManager, SubmitItemUpdateResult_t> m_SteamCallSubmitItem;

	FString PendingUploadWorldName;
	FString PendingUploadWorkshopItemName;
	FString PendingUploadWorkshopItemDescription;

};
