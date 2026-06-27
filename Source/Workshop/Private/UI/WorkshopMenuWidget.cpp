// Copyright Telephone Studios. All Rights Reserved.


#include "UI/WorkshopMenuWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Log.h"

UWorkshopMenuWidget::UWorkshopMenuWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UWorkshopMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

}