// Copyright Telephone Studios. All Rights Reserved.


#include "UI/VitalsWidget.h"
#include "Components/ProgressBar.h"
#include "Components/VitalsComponent.h"

void UVitalsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateBars();
}

void UVitalsWidget::UpdateBars()
{
	APawn* OwnerPawn = GetOwningPlayerPawn<APawn>();
	if (OwnerPawn == nullptr)
	{
		return;
	}

	UVitalsComponent* PlayerVitals = GetOwningPlayerPawn<APawn>()->FindComponentByClass<UVitalsComponent>();
	if (PlayerVitals == nullptr)
	{
		return;
	}

	HealthBar->SetPercent(PlayerVitals->GetHealth() / PlayerVitals->GetMaxHealth());
	ThirstBar->SetPercent(PlayerVitals->GetThirst() / PlayerVitals->GetMaxThirst());
	HungerBar->SetPercent(PlayerVitals->GetHunger() / PlayerVitals->GetMaxHunger());
}