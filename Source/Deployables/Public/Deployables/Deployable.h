// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BuildAnchorComponent.h"
#include "Interfaces/SavableObject.h"
#include "Interfaces/RequiredForLoad.h"
#include "Interfaces/DurabilityInterface.h"
#include "Enums/ToolType.h"
#include "Interfaces/DamagedByWind.h"
#include "Deployable.generated.h"

class UNavModifierComponent;

UCLASS()
class DEPLOYABLES_API ADeployable : public AActor, public IDurabilityInterface, public ISavableObject, public IRequiredForLoad, public IDamagedByWind
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADeployable();
	
	virtual void OnSpawn();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Serialize(FArchive& Ar) override;

	virtual void Destroyed() override;

	// Begin IDamagedByWind Implementation
	virtual void ApplyWindDamage(AActor* WindCauser, float DamageMultiplier = 1.0f) override;
	// End IDamagedByWind Implementation

	virtual float TakeDamage(float DamageAmount, const struct FDamageEvent& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UStaticMesh* GetMesh() const;
	FTransform GetMeshTransform() const;

	// Begin IDurabilityInterface Implementation
	virtual float GetCurrentDurability() override;
	virtual float GetMaxDurability() override;
	// End IDurabilityInterface Implementation
	
	// Begin ISavableObject Implementation
	virtual FName GetIdentifier() const override;
	// End ISavableObject Implementation

	TEnumAsByte<EToolType> GetMaterialType();

	bool CanSpawnOnGround() const;
	bool CanSpawnOnFloor() const;
	bool CanSpawnOnWall() const;
	TEnumAsByte<EBuildAnchorType> CanSpawnOnBuildAnchor() const;
	bool FollowsSurfaceNormal() const;
	bool CanRotate() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* DeployableRootComponent;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere)
	UNavModifierComponent* NavigationModifier;

	UPROPERTY(EditDefaultsOnly, Category = "Deployable Sound")
	USoundBase* PlacementSound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Deployable Sound")
	USoundBase* DestructionSound;

	UPROPERTY(EditDefaultsOnly, Category = "Deployable")
	float MaxDurability;

	UPROPERTY(EditDefaultsOnly, Category = "Deployable")
	TEnumAsByte<EToolType> MaterialType;

	UPROPERTY(Replicated)
	float CurrentDurability;

	UPROPERTY(SaveGame)
	float NormalizedDurability;

	UPROPERTY(EditDefaultsOnly, Category = "Save System")
	FName Identifier;

	UPROPERTY(EditDefaultsOnly, Category = "Deployable Placement Settings")
	bool bCanSpawnOnGround;
	UPROPERTY(EditDefaultsOnly, Category = "Deployable Placement Settings")
	bool bCanSpawnOnFloor;
	UPROPERTY(EditDefaultsOnly, Category = "Deployable Placement Settings")
	bool bCanSpawnOnWall;
	UPROPERTY(EditDefaultsOnly, Category = "Deployable Placement Settings")
	TEnumAsByte<EBuildAnchorType> CanSpawnOnAnchor;
	UPROPERTY(EditDefaultsOnly, Category = "Deployable Placement Settings")
	bool bFollowsSurfaceNormal;
	UPROPERTY(EditDefaultsOnly, Category = "Deployable Placement Settings")
	bool bCanRotate;

	UFUNCTION(NetMulticast, Reliable)
	void Multi_PlayPlacementEffects();

	UFUNCTION(NetMulticast, Reliable)
	void Multi_PlayDestructionEffects();

	void SpawnDustEffects();

	UFUNCTION()
	virtual void OnLoadComplete_Implementation() override;

};
