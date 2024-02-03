// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Structs/PlayerSaveData.h"
#include "Structs/ChunkData.h"
#include "Structs/WeatherData.h"
#include "Enums/GameDifficulty.h"
#include "WildOmissionSaveGame.generated.h"

USTRUCT()
struct FWildOmissionSaveCreationInformation
{
	GENERATED_BODY()

	FWildOmissionSaveCreationInformation();

	UPROPERTY()
	FString Name;

	UPROPERTY()
	bool LevelHasGenerated;

	UPROPERTY()
	uint8 Day;
	
	UPROPERTY()
	uint8 Month;
	
	UPROPERTY()
	uint16 Year;

};

UCLASS()
class SAVESYSTEM_API UWildOmissionSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UWildOmissionSaveGame();

	static int32 GetCurrentVersion();

	UPROPERTY()
	uint32 DaysPlayed;
	
	UPROPERTY()
	float NormalizedProgressThroughDay;

	UPROPERTY()
	FWeatherData WeatherData;

	UPROPERTY()
	FDateTime LastPlayedTime;

	UPROPERTY()
	TEnumAsByte<EGameDifficulty> Difficulty;

	UPROPERTY()
	FString LevelFile;

	UPROPERTY()
	uint32 Seed;

	UPROPERTY()
	FWildOmissionSaveCreationInformation CreationInformation;

	UPROPERTY()
	int32 Version;

	UPROPERTY()
	bool CheatsEnabled;

	UPROPERTY()
	TArray<FChunkData> ChunkData;

	UPROPERTY()
	TArray<FPlayerSaveData> PlayerData;

};