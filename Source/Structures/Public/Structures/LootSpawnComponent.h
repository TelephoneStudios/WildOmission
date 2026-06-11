// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Interfaces/SavableObject.h"
#include "LootSpawnComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STRUCTURES_API ULootSpawnComponent : public USceneComponent, public ISavableObject
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULootSpawnComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly)
	float MinSpawnFrequencySeconds;

	UPROPERTY(EditDefaultsOnly)
	float MaxSpawnFrequencySeconds;

	UPROPERTY(EditDefaultsOnly)
	bool UseRandomYawRotation;

	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<class ADeployable>> LootToSpawn;

	UPROPERTY(SaveGame)
	float TimeTillNextSpawnSeconds;
	
	UPROPERTY()
	class ADeployable* CurrentLoot;
		
};
