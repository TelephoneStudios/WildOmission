// Copyright Telephone Studios. All Rights Reserved.


#include "Structs/CustomCharacterData.h"

FLinearColor FCustomCharacterData::GetDefaultSkinColor() const
{
    return FLinearColor(0.552f, 0.428f, 0.371f);
}

FLinearColor FCustomCharacterData::GetDefaultShirtColor() const
{
    return FLinearColor(0.265f, 0.099f, 0.104f);
}

FLinearColor FCustomCharacterData::GetDefaultPantsColor() const
{
    return FLinearColor(0.136f, 0.189f, 0.265f);
}

FLinearColor FCustomCharacterData::GetDefaultShoeColor() const
{
    return FLinearColor(0.296f, 0.172f, 0.035f);
}
