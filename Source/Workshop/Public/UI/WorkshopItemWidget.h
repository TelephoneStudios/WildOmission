// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WorkshopManager.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "ThirdParty/Steamworks/sdk/public/steam/steam_api.h"
#include "WorkshopItemWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorkshopItemButtonClickedSignature, const FSteamWorkshopItemDetails&, Details);

UCLASS()
class WORKSHOP_API UWorkshopItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UWorkshopItemWidget(const FObjectInitializer& ObjectInitializer);

	void Setup(const FSteamWorkshopItemDetails& InDetails);

	FOnWorkshopItemButtonClickedSignature OnClicked;

protected:
	virtual void NativeTick(const FGeometry& MyGeomotry, float InDeltaTime) override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY(Meta = (BindWidget))
	class UButton* Button;
	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* NameTextBlock;
	UPROPERTY(Meta = (BindWidget))
	class UTextBlock* AuthorTextBlock;
	UPROPERTY(Meta = (BindWidget))
	class UImage* PreviewImage;

	FSteamWorkshopItemDetails ItemDetails;

	CCallbackManual<UWorkshopItemWidget, PersonaStateChange_t> m_personaCallback;

	void FetchAuthorName(CSteamID SteamID);
	void OnPersonaStateChange(PersonaStateChange_t* pParam);

	// Preview Image
	void DownloadPreviewTexture(const FString& URL);
	void OnPreviewDownloaded(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	UFUNCTION()
	void OnButtonClicked();

};
