// Copyright Telephone Studios. All Rights Reserved.


#include "WorldRowWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Color/UIColors.h"

UWorldRowWidget::UWorldRowWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	Icon = nullptr;
	RowButton = nullptr;
	WorldNameTextBlock = nullptr;
	DateCreatedTextBlock = nullptr;
	DaysPlayedTextBlock = nullptr;
	WorldName = TEXT("");
	Selected = false;
}

void UWorldRowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RowButton->OnClicked.AddDynamic(this, &UWorldRowWidget::BroadcastOnClicked);
}

void UWorldRowWidget::Setup(const FWorldRowInformation& InInformation)
{
	WorldName = InInformation.Name;

	const FString DaysPlayedString = FString::Printf(TEXT("%i Days"), InInformation.DaysPlayed);
	const FString CreationString = FString::Printf(TEXT("Created: %i/%i/%i"), InInformation.CreationMonth, InInformation.CreationDay, InInformation.CreationYear);

	if (InInformation.Icon)
	{
		Icon->SetBrushFromTexture(InInformation.Icon);
	}
	else
	{
		Icon->SetVisibility(ESlateVisibility::Collapsed);
	}

	WorldNameTextBlock->SetText(FText::FromString(InInformation.Name));
	DateCreatedTextBlock->SetText(FText::FromString(CreationString));
	DaysPlayedTextBlock->SetText(FText::FromString(DaysPlayedString));
}

void UWorldRowWidget::SetSelected(bool InSelected)
{
	Selected = InSelected;
}

FString UWorldRowWidget::GetWorldName() const
{
	return WorldName;
}

void UWorldRowWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	FUIColor* Blue = UUIColors::GetBaseColor(TEXT("Blue"));
	FUIColor* LightGray = UUIColors::GetBaseColor(TEXT("LightGray"));
	if (Blue == nullptr || LightGray == nullptr)
	{
		return;
	}

	FLinearColor ButtonColor = Selected ? Blue->Default : LightGray->Default;
	RowButton->SetBackgroundColor(ButtonColor * FLinearColor(1.0f, 1.0f, 1.0f, 0.5f));
}

void UWorldRowWidget::BroadcastOnClicked()
{
	if (!OnClicked.IsBound())
	{
		return;
	}

	OnClicked.Broadcast(WorldName);
}

FWorldRowInformation::FWorldRowInformation()
{
	DaysPlayed = 0;
	CreationMonth = 0;
	CreationDay = 0;
	CreationYear = 0;
	Name = TEXT("");
}

FWorldRowInformation::FWorldRowInformation(UTexture2D* InIcon, const uint32& InDaysPlayed, const uint8& InCreationMonth, const uint8& InCreationDay, const uint16& InCreationYear, const FString& InName)
{
	Icon = InIcon;
	DaysPlayed = InDaysPlayed;
	CreationMonth = InCreationMonth;
	CreationDay = InCreationDay;
	CreationYear = InCreationYear;
	Name = InName;
}