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
	FLinearColor SkinColor = FLinearColor::White;
	
	UPROPERTY()
	FLinearColor ShirtColor = FLinearColor::White;
	
	UPROPERTY()
	FLinearColor PantsColor = FLinearColor::White;
	
	UPROPERTY()
	FLinearColor ShoeColor = FLinearColor::White;

};