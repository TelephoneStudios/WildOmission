// Copyright Telephone Studios. All Rights Reserved.

#include "Components/CustomCharacterComponent.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Net/UnrealNetwork.h"

UCustomCharacterComponent::UCustomCharacterComponent()
{
	AffectingMeshComponent = nullptr;
	CharacterMaterial = nullptr;
	DynamicMaterial = nullptr;
	MaleMesh = nullptr;
	FemaleMesh = nullptr;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> CharacterMaterialObject(TEXT("/Game/WildOmissionCore/Art/Characters/M_HumanClothing_Inst"));
	if (CharacterMaterialObject.Succeeded())
	{
		CharacterMaterial = CharacterMaterialObject.Object;
	}
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

void UCustomCharacterComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCustomCharacterComponent, CurrentData);
}

void UCustomCharacterComponent::Setup(USkeletalMeshComponent* InMeshComponent)
{
	AffectingMeshComponent = InMeshComponent;
	
	if (CharacterMaterial)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(CharacterMaterial, this);
		AffectingMeshComponent->SetMaterial(0, DynamicMaterial);
	}

	Apply(LoadData());
}

void UCustomCharacterComponent::Apply(const FCustomCharacterData& InCharacterData)
{
	CurrentData = InCharacterData;

	if (!GetOwner()->HasAuthority())
	{
		Server_UpdateCharacterData(InCharacterData);
	}

	if (AffectingMeshComponent == nullptr)
	{
		return;
	}

	AffectingMeshComponent->SetSkeletalMesh(
		InCharacterData.bIsFemale ? FemaleMesh : MaleMesh
	);

	DynamicMaterial->SetVectorParameterValue(TEXT("Skin Color"), FVector4(InCharacterData.SkinColor.R,
		InCharacterData.SkinColor.G, InCharacterData.SkinColor.B, 1.0f));
	DynamicMaterial->SetVectorParameterValue(TEXT("Shirt Color"), FVector4(InCharacterData.ShirtColor.R,
		InCharacterData.ShirtColor.G, InCharacterData.ShirtColor.B, 1.0f));
	DynamicMaterial->SetVectorParameterValue(TEXT("Pants Color"), FVector4(InCharacterData.PantsColor.R,
		InCharacterData.PantsColor.G, InCharacterData.PantsColor.B, 1.0f));
	DynamicMaterial->SetVectorParameterValue(TEXT("Shoe Color"), FVector4(InCharacterData.ShoeColor.R,
		InCharacterData.ShoeColor.G, InCharacterData.ShoeColor.B, 1.0f));

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

void UCustomCharacterComponent::ResetDefaults()
{
	SaveData(FCustomCharacterData());
}

void UCustomCharacterComponent::OnRep_CurrentData()
{
	this->Apply(CurrentData);
}

void UCustomCharacterComponent::Server_UpdateCharacterData_Implementation(const FCustomCharacterData& InCharacterData)
{
	Apply(InCharacterData);
}
