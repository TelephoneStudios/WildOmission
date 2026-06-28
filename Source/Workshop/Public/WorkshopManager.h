// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#pragma warning(disable: 4996)
#include "ThirdParty/Steamworks/sdk/public/steam/steam_api.h"
#include "WorkshopManager.generated.h"

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
	
	static bool IsItemUpdateComplete(const int64& InUpdateHandle);

private:
	// Handler for the asynchronous call result
	void OnItemCreated(struct CreateItemResult_t* pCallback, bool bIOFailure);
	// Steam call result wrapper
	CCallResult<UWorkshopManager, CreateItemResult_t> m_SteamCallResultCreateItem;

};
