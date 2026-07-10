// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Structs/CustomCharacterData.h"
#include "CustomCharacterComponent.generated.h"

UCLASS()
class CHARACTERCUSTOMIZATION_API UCustomCharacterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCustomCharacterComponent();

	void Setup(class USkeletalMeshComponent* InMeshComponent);

	void Apply(const FCustomCharacterData& InCharacterData);

	static void SaveData(const FCustomCharacterData& InCharacterData);
	static FCustomCharacterData LoadData();
protected:
	class USkeletalMeshComponent* AffectingMeshComponent;

	UPROPERTY(EditDefaultsOnly)
	class USkeletalMesh* MaleMesh;
	UPROPERTY(EditDefaultsOnly)
	class USkeletalMesh* FemaleMesh;

};