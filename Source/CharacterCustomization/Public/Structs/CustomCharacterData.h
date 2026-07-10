// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CustomCharacterData.generated.h"

USTRUCT()
struct CHARACTERCUSTOMIZATION_API FCustomCharacterData
{
	GENERATED_BODY()

	UPROPERTY()
	bool bIsFemale = false;

	UPROPERTY()
	FColor SkinColor;
	
	UPROPERTY()
	FColor ShirtColor;
	
	UPROPERTY()
	FColor PantsColor;
	
	UPROPERTY()
	FColor ShoeColor;

};