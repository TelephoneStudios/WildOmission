// Copyright Telephone Studios. All Rights Reserved.


#include "Components/PlayerSaveManagerComponent.h"
#include "WorldData.h"
#include "Interfaces/SavablePlayer.h"
#include "SaveManager.h"
#include "Structs/PlayerSaveData.h"
#include "Log.h"

// Sets default values for this component's properties
UPlayerSaveManagerComponent::UPlayerSaveManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

}

void UPlayerSaveManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	FTimerHandle UpdatePendingTimerHandle;
	World->GetTimerManager().SetTimer(UpdatePendingTimerHandle, this, &UPlayerSaveManagerComponent::AddAllToPending, 60.0f, true);
}

void UPlayerSaveManagerComponent::AddToPending(APlayerController* PlayerController)
{
	ISavablePlayer* SavablePlayer = Cast<ISavablePlayer>(PlayerController);
	if (SavablePlayer == nullptr)
	{
		return;
	}

	AddSaveToList(SavablePlayer->SavePlayer(), PendingSaves);
}

void UPlayerSaveManagerComponent::Save(TArray<FPlayerSaveData>& OutUpdatedSavesList)
{
	for (const FPlayerSaveData& InSave : PendingSaves)
	{
		AddSaveToList(InSave, OutUpdatedSavesList);
	}

	PendingSaves.Empty();
}

void UPlayerSaveManagerComponent::Load(APlayerController* PlayerController)
{
	ASaveManager* SaveManagerOwner = Cast<ASaveManager>(GetOwner());
	ISavablePlayer* SavablePlayer = Cast<ISavablePlayer>(PlayerController);
	if (SaveManagerOwner == nullptr || SavablePlayer == nullptr)
	{
		UE_LOG(LogSaveSystem, Error, TEXT("Couldn't load the player, SaveManager or PlayerController was nullptr."));
		return;
	}

	int32 SaveIndex = 0;
	const FString PlayerUniqueID = SavablePlayer->GetUniqueID();
	const bool PlayerIsHost = SavablePlayer->IsHost();

	UWorldData* WorldData = SaveManagerOwner->GetWorldData();
	if (WorldData == nullptr)
	{
		return;
	}

	// Attempt host parody
	if (PlayerIsHost && FindHostSaveIndexInList(WorldData->PlayerData, SaveIndex))
	{
		SavablePlayer->LoadPlayerSave(WorldData->PlayerData[SaveIndex]);
		WorldData->PlayerData[SaveIndex].IsHost = false;
		SaveManagerOwner->UpdateWorldData(WorldData);
		return;
	}

	// Find existing save in the pending list
	if (FindSaveIndexInList(PendingSaves, PlayerUniqueID, SaveIndex))
	{
		SavablePlayer->LoadPlayerSave(PendingSaves[SaveIndex]);
	}
	// Find existing save in the save file
	else if (FindSaveIndexInList(WorldData->PlayerData, PlayerUniqueID, SaveIndex))
	{
		SavablePlayer->LoadPlayerSave(WorldData->PlayerData[SaveIndex]);
	}
}

void UPlayerSaveManagerComponent::AddAllToPending()
{
	for (APlayerController* PlayerController : GetAllPlayerControllers())
	{
		ISavablePlayer* SavablePlayer = Cast<ISavablePlayer>(PlayerController);
		if (SavablePlayer == nullptr || SavablePlayer->IsStillLoading())
		{
			continue;
		}

		AddToPending(PlayerController);
	}
}

void UPlayerSaveManagerComponent::AddSaveToList(const FPlayerSaveData& InSave, TArray<FPlayerSaveData>& OutSavesList)
{
	int32 SaveIndex = 0;
	if (FindSaveIndexInList(OutSavesList, InSave.UniqueID, SaveIndex))
	{
		OutSavesList[SaveIndex] = InSave;
	}
	else
	{
		OutSavesList.Add(InSave);
	}
}

bool UPlayerSaveManagerComponent::FindSaveIndexInList(const TArray<FPlayerSaveData>& SaveList, const FString& UniqueID, int32& OutIndex) const
{
	int32 Index = 0;
	bool SaveFound = false;
	for (const FPlayerSaveData& Save : SaveList)
	{
		if (Save.UniqueID != UniqueID)
		{
			++Index;
			continue;
		}

		SaveFound = true;
		break;
	}

	OutIndex = Index;
	return SaveFound;
}

bool UPlayerSaveManagerComponent::FindHostSaveIndexInList(const TArray<FPlayerSaveData>& SaveList, int32& OutIndex) const
{
	int32 Index = 0;
	bool SaveFound = false;
	for (const FPlayerSaveData& Save : SaveList)
	{
		if (Save.IsHost == false)
		{
			++Index;
			continue;
		}

		SaveFound = true;
		break;
	}

	OutIndex = Index;
	return SaveFound;
}

TArray<APlayerController*> UPlayerSaveManagerComponent::GetAllPlayerControllers()
{
	TArray<APlayerController*> PlayerControllers;

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return PlayerControllers;
	}

	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		PlayerControllers.Add(Iterator->Get());
	}

	return PlayerControllers;
}