// Copyright Telephone Studios. All Rights Reserved.


#include "Items/MonsterArmsItem.h"
#include "Monsters/Monster.h"
#include "SaveManager.h"
#include "WorldInformation.h"
#include "Components/EquipComponent.h"
#include "Engine/DamageEvents.h"

AMonsterArmsItem::AMonsterArmsItem()
{
	DamageMultiplier = 0.3f;
}

void AMonsterArmsItem::Equip(APawn* InOwnerPawn, USkeletalMeshComponent* InThirdPersonMeshComponent, const FName& InItemName, const int8& InFromSlotIndex, const uint32& InUniqueID)
{
	Super::Equip(InOwnerPawn, InThirdPersonMeshComponent, InItemName, InFromSlotIndex, InUniqueID);

	// Set damage multiplayer depending on difficulty
	ASaveManager* SaveManager = ASaveManager::GetSaveManager();
	if (SaveManager == nullptr)
	{
		return;
	}

	UWorldInformation* WorldInformation = SaveManager->GetWorldInformation();
	if (WorldInformation == nullptr)
	{
		return;
	}

	// 0 = peace, 1 = easy, 2 = normal, 3 = hard
	const int8 DifficultyModifier = (WorldInformation->Difficulty.GetIntValue() - 2) * 0.15f;
	DamageMultiplier = FMath::Clamp(DamageMultiplier + DifficultyModifier, 0.1f, 1.0f);
}

void AMonsterArmsItem::OnPrimaryHeld()
{
	Super::OnPrimaryHeld();

	Swing();
}
