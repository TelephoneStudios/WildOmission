// Copyright Telephone Studios. All Rights Reserved.

#include "Actors/CustomCharacter.h"
#include "Components/CustomCharacterComponent.h"

ACustomCharacter::ACustomCharacter()
{
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	if (Mesh == nullptr)
	{
		return;
	}

	RootComponent = Mesh;

	CustomCharacterComponent = CreateDefaultSubobject<UCustomCharacterComponent>(TEXT("CustomCharacterComponent"));
}

void ACustomCharacter::BeginPlay()
{
	Super::BeginPlay();
	CustomCharacterComponent->Setup(Mesh);
}

void ACustomCharacter::Destroyed()
{
	Super::Destroyed();
}