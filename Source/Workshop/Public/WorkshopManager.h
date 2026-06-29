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

	void UploadWorkshopItem();
	
	float GetItemUploadPercentage() const;

	static bool IsItemUpdateComplete(const int64& InUpdateHandle);

	FOnWorkshopItemSubmittedSignature OnWorkshopItemSubmitted;

private:
	// Handler for the asynchronous call result
	void OnItemCreated(struct CreateItemResult_t* pCallback, bool bIOFailure);
	void WorkshopSubmittedCallback(struct SubmitItemUpdateResult_t* pCallback, bool bIOFailure);
	void UploadItemContent(PublishedFileId_t nFileID);

	// Steam call result wrapper
	CCallResult<UWorkshopManager, CreateItemResult_t> m_SteamCallResultCreateItem;
	CCallResult<UWorkshopManager, SubmitItemUpdateResult_t> m_SteamCallSubmitItem;

};
