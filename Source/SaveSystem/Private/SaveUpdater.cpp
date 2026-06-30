// Copyright Telephone Studios. All Rights Reserved.


#include "SaveUpdater.h"
#include "Components/PlayerSaveManagerComponent.h"
#include "ChunkManager.h"
#include "TimeOfDayManager.h"
#include "WeatherManager.h"
#include "Interfaces/GameSaveLoadController.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "WildOmissionSaveGame.h"
#include "WorldInformation.h"
#include "WorldData.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Log.h"

static USaveUpdater* Instance = nullptr;

// Sets default values
USaveUpdater::USaveUpdater()
{

}

void USaveUpdater::UpdateWorldFiles(TArray<FString> OldWorldNames)
{
	for (const FString& OldWorldName : OldWorldNames)
	{
		// Get legacy save game
		UWildOmissionSaveGame* LegacySaveGame = Cast<UWildOmissionSaveGame>(UGameplayStatics::CreateSaveGameObject(UWildOmissionSaveGame::StaticClass()));
		LegacySaveGame = Cast<UWildOmissionSaveGame>(UGameplayStatics::LoadGameFromSlot(OldWorldName, 0));
		if (LegacySaveGame == nullptr)
		{
			UE_LOG(LogSaveSystem, Warning, TEXT("Couldn't update legacy save file to new world file, failed to load legacy save game."));
			continue;
		}

		// Create new world save
		UWorldInformation* WorldInformation = Cast<UWorldInformation>(UGameplayStatics::CreateSaveGameObject(UWorldInformation::StaticClass()));
		UWorldData* WorldData = Cast<UWorldData>(UGameplayStatics::CreateSaveGameObject(UWorldData::StaticClass()));
		if (WorldInformation == nullptr || WorldData == nullptr)
		{
			UE_LOG(LogSaveSystem, Warning, TEXT("Couldn't update legacy save file to new world file, failed to create new save objects."));
			continue;
		}

		// Populate world information
		WorldInformation->Version = LegacySaveGame->Version;
		WorldInformation->DaysPlayed = LegacySaveGame->DaysPlayed;
		WorldInformation->NormalizedTimeOfDay = LegacySaveGame->NormalizedTimeOfDay;
		WorldInformation->LastPlayedTime = LegacySaveGame->LastPlayedTime;
		WorldInformation->Difficulty = LegacySaveGame->Difficulty;
		WorldInformation->LevelFile = LegacySaveGame->LevelFile;
		WorldInformation->Seed = LegacySaveGame->Seed;
		WorldInformation->CreationInformation.Name = LegacySaveGame->CreationInformation.Name;
		WorldInformation->CreationInformation.LevelHasGenerated = LegacySaveGame->CreationInformation.LevelHasGenerated;
		WorldInformation->CreationInformation.Day = LegacySaveGame->CreationInformation.Day;
		WorldInformation->CreationInformation.Month = LegacySaveGame->CreationInformation.Month;
		WorldInformation->CreationInformation.Year = LegacySaveGame->CreationInformation.Year;
		WorldInformation->CheatsEnabled = LegacySaveGame->CheatsEnabled;
		WorldInformation->GameMode = LegacySaveGame->GameMode;

		// Populate world data
		WorldData->WeatherData = LegacySaveGame->WeatherData;
		WorldData->PlayerSpawnChunk = LegacySaveGame->PlayerSpawnChunk;
		WorldData->PlayerSpawnOverride = LegacySaveGame->PlayerSpawnOverride;
		WorldData->ChunkData = LegacySaveGame->ChunkData;
		WorldData->PlayerData = LegacySaveGame->PlayerData;
	
		// Save the world
		UGameplayStatics::SaveGameToSlot(WorldInformation, OldWorldName + TEXT("/WorldInformation"), 0);
		UGameplayStatics::SaveGameToSlot(WorldData, OldWorldName + TEXT("/WorldData"), 0);

		// Delete the old save game
		UGameplayStatics::DeleteGameInSlot(OldWorldName, 0);
	}
}