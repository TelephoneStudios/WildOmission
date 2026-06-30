// Copyright Telephone Studios. All Rights Reserved.


#include "WorldSelectionWidget.h"
#include "WorldRowWidget.h"
#include "Components/PanelWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "SaveManager.h"
#include "WorldInformation.h"
#include "Kismet/GameplayStatics.h"

UWorldSelectionWidget::UWorldSelectionWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	WorldListBox = nullptr;
	SelectButton = nullptr;
	CancelButton = nullptr;

	WorldRowWidgetClass = nullptr;

	static ConstructorHelpers::FClassFinder<UWorldRowWidget> WorldRowWidgetBPClass(TEXT("/Game/MenuSystem/UI/World/WBP_WorldRow"));
	if (WorldRowWidgetBPClass.Succeeded())
	{
		WorldRowWidgetClass = WorldRowWidgetBPClass.Class;
	}
}

void UWorldSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SelectButton->SetIsEnabled(false);
	SelectButton->OnClicked.AddDynamic(this, &UWorldSelectionWidget::BroadcastSelectButtonClicked);
	CancelButton->OnClicked.AddDynamic(this, &UWorldSelectionWidget::BroadcastCancelButtonClicked);
}

void UWorldSelectionWidget::SetWorldList(const TArray<FString>& WorldNames)
{
	SelectedWorldName.Reset();

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	WorldListBox->ClearChildren();

	TArray<UWorldInformation*> SortedWorlds = GetWorldsSortedByLastPlayed(WorldNames);

	for (UWorldInformation* WorldInformation : SortedWorlds)
	{
		if (WorldInformation == nullptr)
		{
			continue;
		}

		UWorldRowWidget* Row = CreateWidget<UWorldRowWidget>(World, WorldRowWidgetClass);
		if (Row == nullptr)
		{
			return;
		}

		UTexture2D* Icon = ASaveManager::GetWorldIcon(WorldInformation->CreationInformation.Name);

		const FWorldRowInformation Information(
			Icon,
			WorldInformation->DaysPlayed,
			WorldInformation->CreationInformation.Month,
			WorldInformation->CreationInformation.Day,
			WorldInformation->CreationInformation.Year,
			WorldInformation->CreationInformation.Name
		);
		Row->Setup(Information);
		Row->OnClicked.AddDynamic(this, &UWorldSelectionWidget::SetSelectedWorld);
		WorldListBox->AddChild(Row);
	}
}

void UWorldSelectionWidget::SetSelectedWorld(const FString& WorldName)
{
	SelectedWorldName = WorldName;
	UpdateListChildren();
}

void UWorldSelectionWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	SelectButton->SetIsEnabled(SelectedWorldName.IsSet());
}

void UWorldSelectionWidget::UpdateListChildren()
{
	for (int32 i = 0; i < WorldListBox->GetChildrenCount(); ++i)
	{
		UWorldRowWidget* Row = Cast<UWorldRowWidget>(WorldListBox->GetChildAt(i));

		if (Row == nullptr)
		{
			continue;
		}

		const bool RowSelected = (SelectedWorldName.IsSet() && SelectedWorldName.GetValue() == Row->GetWorldName());
		Row->SetSelected(RowSelected);
	}
}

TArray<UWorldInformation*> UWorldSelectionWidget::GetWorldsSortedByLastPlayed(const TArray<FString>& NamesList)
{
	TArray<UWorldInformation*> SortedWorlds;
	for (const FString& WorldName : NamesList)
	{
		FString InformationDirectory = WorldName + TEXT("/WorldInformation");
		UWorldInformation* WorldInformation = Cast<UWorldInformation>(UGameplayStatics::LoadGameFromSlot(InformationDirectory, 0));
		if (WorldInformation == nullptr)
		{
			continue;
		}

		SortedWorlds.Add(WorldInformation);
	}

	Algo::Sort(SortedWorlds, IsSaveMoreRecentlyPlayed);

	return SortedWorlds;
}

bool UWorldSelectionWidget::IsSaveMoreRecentlyPlayed(UWorldInformation* WorldA, UWorldInformation* WorldB)
{
	return WorldA->LastPlayedTime > WorldB->LastPlayedTime;
}

void UWorldSelectionWidget::BroadcastSelectButtonClicked()
{
	if (!OnSelectButtonClicked.IsBound())
	{
		return;
	}

	OnSelectButtonClicked.Broadcast();
}

void UWorldSelectionWidget::BroadcastCancelButtonClicked()
{
	if (!OnCancelButtonClicked.IsBound())
	{
		return;
	}

	OnCancelButtonClicked.Broadcast();
}
