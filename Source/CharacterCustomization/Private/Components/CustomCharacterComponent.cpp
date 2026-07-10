// Copyright Telephone Studios. All Rights Reserved.

#include "Components/CustomCharacterComponent.h"

UCustomCharacterComponent::UCustomCharacterComponent()
{
	AffectingMeshComponent = nullptr;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MaleSkeletalMeshObject(TEXT("/Game/WildOmissionCore/Art/Characters/SK_Human_Male"));
	if (MaleSkeletalMeshObject.Succeeded())
	{
		MaleMesh = MaleSkeletalMeshObject.Object;
	}
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> FemaleSkeletalMeshObject(TEXT("/Game/WildOmissionCore/Art/Characters/SK_Human_Female"));
	if (FemaleSkeletalMeshObject.Succeeded())
	{
		FemaleMesh = FemaleSkeletalMeshObject.Object;
	}

}

void UCustomCharacterComponent::Setup(USkeletalMeshComponent* InMeshComponent)
{
	AffectingMeshComponent = InMeshComponent;
	Apply(LoadData());
}

void UCustomCharacterComponent::Apply(const FCustomCharacterData& InCharacterData)
{
	if (AffectingMeshComponent == nullptr)
	{
		return;
	}

	AffectingMeshComponent->SetSkeletalMesh(
		InCharacterData.bIsFemale ? FemaleMesh : MaleMesh
	);

	// TODO clothing
}

void UCustomCharacterComponent::SaveData(const FCustomCharacterData& InCharacterData)
{

}

FCustomCharacterData UCustomCharacterComponent::LoadData()
{
	return FCustomCharacterData();
}