// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WorkshopMenuWidget.generated.h"

UCLASS()
class WORKSHOP_API UWorkshopMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UWorkshopMenuWidget(const FObjectInitializer& ObjectInitializer);
	
	virtual void NativeConstruct() override;

private:

};
