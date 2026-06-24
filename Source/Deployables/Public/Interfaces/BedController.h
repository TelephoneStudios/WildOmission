// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BedController.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UBedController : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DEPLOYABLES_API IBedController
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void Sleep() = 0;
	virtual void SetCurrentBed(const int32& InBedUniqueID, const FIntVector2& InBedChunkLocation) = 0;
	virtual int32 GetBedUniqueID() const = 0;
	virtual FIntVector2 GetBedChunkLocation() const = 0;

};
