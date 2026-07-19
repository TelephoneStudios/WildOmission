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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void Setup(class USkeletalMeshComponent* InMeshComponent);

	void Apply(const FCustomCharacterData& InCharacterData);

	static void SaveData(const FCustomCharacterData& InCharacterData);
	static FCustomCharacterData LoadData();
protected:
	class USkeletalMeshComponent* AffectingMeshComponent;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_CurrentData)
	FCustomCharacterData CurrentData;
	
	UPROPERTY(EditDefaultsOnly)
	class USkeletalMesh* MaleMesh;
	UPROPERTY(EditDefaultsOnly)
	class USkeletalMesh* FemaleMesh;

	UPROPERTY(EditDefaultsOnly)
	class UMaterialInterface* CharacterMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;

	UFUNCTION()
	void OnRep_CurrentData();

	UFUNCTION(Server, Reliable)
	void Server_UpdateCharacterData(const FCustomCharacterData& InCharacterData);

};