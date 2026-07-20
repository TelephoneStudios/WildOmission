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
	FLinearColor SkinColor = GetDefaultSkinColor();
	
	UPROPERTY()
	FLinearColor ShirtColor = GetDefaultShirtColor();
	
	UPROPERTY()
	FLinearColor PantsColor = GetDefaultPantsColor();
	
	UPROPERTY()
	FLinearColor ShoeColor = GetDefaultShoeColor();

	FLinearColor GetDefaultSkinColor() const;
	FLinearColor GetDefaultShirtColor() const;
	FLinearColor GetDefaultPantsColor() const;
	FLinearColor GetDefaultShoeColor() const;

};