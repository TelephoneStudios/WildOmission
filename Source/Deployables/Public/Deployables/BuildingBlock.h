// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Deployables/Deployable.h"
#include "BuildingBlock.generated.h"

UENUM()
enum EBuildMaterial
{
	// Wood is for trees.
	BM_WOOD	UMETA(DisplayName = "Wood"),
	// Stone is for nodes.
	BM_STONE	UMETA(DisplayName = "Stone"),
	// Unused as far as I know.
	BM_METAL	UMETA(DisplayName = "Metal"),
};

UCLASS()
class DEPLOYABLES_API ABuildingBlock : public ADeployable
{
	GENERATED_BODY()
	
public:
	ABuildingBlock();
	void Upgrade();
	bool IsUpgradable() const;

	TEnumAsByte<EBuildMaterial> GetMaterialType();

	ABuildingBlock* GetUpgradeDefaultClass() const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Deployable")
	TEnumAsByte<EBuildMaterial> MaterialType;

	UPROPERTY(EditDefaultsOnly, Category = "Deployable")
	TSubclassOf<ABuildingBlock> UpgradedTier;
};
