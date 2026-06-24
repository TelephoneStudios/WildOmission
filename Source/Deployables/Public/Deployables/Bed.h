// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Deployables/Deployable.h"
#include "Interfaces/Interactable.h"
#include "Bed.generated.h"

/**
 * 
 */
UCLASS()
class DEPLOYABLES_API ABed : public ADeployable, public IInteractable
{
	GENERATED_BODY()
	
public:
	ABed();

	virtual void OnSpawn() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Begin IInteractable Implementation
	virtual void Interact(AActor* Interactor) override;
	virtual FString PromptText() override;
	// End IInteractable Implementation

	USceneComponent* GetSpawnPointComponent() const;

	int32 GetUniqueID() const;

private:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* SpawnPointComponent;
	
	UPROPERTY(VisibleInstanceOnly, SaveGame, Replicated)
	int32 UniqueID;

};
