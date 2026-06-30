// Copyright Telephone Studios. All Rights Reserved.


#include "WorldSaveData.h"

UWorldSaveData::UWorldSaveData()
{
	NormalizedTimeOfDay = 0.0f;
	PlayerSpawnChunk = FPlayerSpawnChunk();
	PlayerSpawnOverride = FPlayerSpawnOverride();
}
