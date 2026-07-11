// Copyright Telephone Studios. All Rights Reserved.

#include "Components/CustomCharacterComponent.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

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
	FString JsonString;

	if (FJsonObjectConverter::UStructToJsonObjectString(InCharacterData, JsonString))
	{
		const FString SaveDir = FPaths::ProjectSavedDir() + TEXT("CustomCharacter/CharacterData.json");
		FFileHelper::SaveStringToFile(JsonString, *SaveDir);
	}
}

FCustomCharacterData UCustomCharacterComponent::LoadData()
{
	const FString LoadDir = FPaths::ProjectSavedDir() + TEXT("CustomCharacter/CharacterData.json");
	FString JsonString;
	FCustomCharacterData Data;
	if (FFileHelper::LoadFileToString(JsonString, *LoadDir))
	{
		(void*)FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &Data, 0, 0);
	}
	return Data;
}