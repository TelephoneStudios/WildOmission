// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PlayerSpawnOverride.generated.h"

USTRUCT()
struct WORLDGENERATION_API FPlayerSpawnOverride
{
	GENERATED_BODY()

	FPlayerSpawnOverride();

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	bool IsSet;

};