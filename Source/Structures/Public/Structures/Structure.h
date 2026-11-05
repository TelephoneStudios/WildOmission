// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/SavableObject.h"
#include "Structure.generated.h"

UCLASS()
class WORLDGENERATION_API AStructure : public AActor, public ISavableObject
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AStructure();
	
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const;
	
	// Begin ISavableObject Implementation
	virtual FName GetIdentifier() const override;
	// End ISavableObject Implementation

private:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* StructureRootComponent;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;
	
	UPROPERTY(EditDefaultsOnly)
	FName Identifier;

	// TODO will have stuff like loot sockets

	// TODO every once and a while we will refresh our loot here, and spawn some good stuff ;)

};