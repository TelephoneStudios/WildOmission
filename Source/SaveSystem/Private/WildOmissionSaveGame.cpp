// Copyright Telephone Studios. All Rights Reserved.


#include "WildOmissionSaveGame.h"

const static int32 CURRENT_SAVE_FILE_VERSION = 2;

FWildOmissionSaveCreationInformation::FWildOmissionSaveCreationInformation()
{
	Name = TEXT("");
	LevelHasGenerated = false;
	Day = 1;
	Month = 1;
	Year = 2023;
}

UWildOmissionSaveGame::UWildOmissionSaveGame()
{
	DaysPlayed = 0;
	NormalizedTimeOfDay = 0.0f;
	LastPlayedTime = FDateTime::Now();
	Difficulty = EGameDifficulty::EGD_Normal;
	LevelFile = TEXT("LV_Procedural");
	Seed = 0;
	Version = 3;
	CheatsEnabled = false;
	GameMode = 0;
	PlayerSpawnChunk = FPlayerSpawnChunk();
}

int32 UWildOmissionSaveGame::GetCurrentVersion()
{
	return CURRENT_SAVE_FILE_VERSION;
}
