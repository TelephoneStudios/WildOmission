// Copyright Telephone Studios. All Rights Reserved.


#include "WorldInformation.h"

const static int32 CURRENT_SAVE_FILE_VERSION = 3;

FWorldCreationInformation::FWorldCreationInformation()
{
	Name = TEXT("");
	LevelHasGenerated = false;

	// This is when V3 world files were created
	Day = 29;
	Month = 6;
	Year = 2026;
}

UWorldInformation::UWorldInformation()
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
}

int32 UWorldInformation::GetCurrentVersion()
{
	return CURRENT_SAVE_FILE_VERSION;
}
