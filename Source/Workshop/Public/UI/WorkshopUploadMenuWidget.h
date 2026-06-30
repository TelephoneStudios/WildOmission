// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WorkshopUploadMenuWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorkshopUploadMenuCancelButtonClickedSignature);

UCLASS()
class WORKSHOP_API UWorkshopUploadMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UWorkshopUploadMenuWidget(const FObjectInitializer& ObjectInitializer);
	
	virtual void NativeConstruct() override;
	
	void SetWorld(const FString& InWorldName);

	FOnWorkshopUploadMenuCancelButtonClickedSignature OnCancelButtonClicked;

protected:
	virtual void NativeTick(const FGeometry& MyGeomotry, float InDeltaTime) override;

private:
	
	UPROPERTY(Meta = (BindWidget))
	class UButton* UploadButton;
	
	UPROPERTY(Meta = (BindWidget))
	class UButton* CancelButton;

	FString WorldName;

	UFUNCTION()
	void UploadButtonClicked();
	UFUNCTION()
	void CancelButtonClicked();
};
