// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Structs/PlayerSaveData.h"
#include "Structs/ChunkData.h"
#include "Structs/PlayerSpawnChunk.h"
#include "Structs/PlayerSpawnOverride.h"
#include "Structs/WeatherData.h"
#include "Enums/GameDifficulty.h"
#include "WorldData.generated.h"

UCLASS()
class SAVESYSTEM_API UWorldData : public USaveGame
{
	GENERATED_BODY()

public:
	UWorldData();

	UPROPERTY()
	float NormalizedTimeOfDay;

	UPROPERTY()
	FWeatherData WeatherData;

	UPROPERTY()
	FPlayerSpawnChunk PlayerSpawnChunk;

	UPROPERTY()
	FPlayerSpawnOverride PlayerSpawnOverride;
	
	UPROPERTY()
	TArray<FChunkData> ChunkData;

	UPROPERTY()
	TArray<FPlayerSaveData> PlayerData;

};