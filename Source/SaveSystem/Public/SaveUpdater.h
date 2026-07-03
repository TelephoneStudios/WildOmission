// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SaveUpdater.generated.h"

UCLASS()
class SAVESYSTEM_API USaveUpdater : public UObject
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	USaveUpdater();

	static void UpdateWorldFiles(TArray<FString> OldWorldNames);
};