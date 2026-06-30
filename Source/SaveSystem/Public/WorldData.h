// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Enums/GameDifficulty.h"
#include "WorldData.generated.h"

USTRUCT()
struct FWorldCreationInformation
{
	GENERATED_BODY()

	FWorldCreationInformation();

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
class SAVESYSTEM_API UWorldData : public USaveGame
{
	GENERATED_BODY()

public:
	UWorldData();

	static int32 GetCurrentVersion();

	UPROPERTY()
	uint32 DaysPlayed;
	
	UPROPERTY()
	FDateTime LastPlayedTime;

	UPROPERTY()
	TEnumAsByte<EGameDifficulty> Difficulty;

	UPROPERTY()
	FString LevelFile;

	UPROPERTY()
	uint32 Seed;

	UPROPERTY()
	FWorldCreationInformation CreationInformation;

	UPROPERTY()
	int32 Version;

	UPROPERTY()
	bool CheatsEnabled;

	UPROPERTY()
	uint8 GameMode;

};