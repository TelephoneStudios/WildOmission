// Copyright Telephone Studios. All Rights Reserved.


#include "Structures/Structure.h"
#include "ChunkManager.h"

AStructure::AStructure()
{
	bReplicates = true;

	StructureRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = StructureRootComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(StructureRootComponent);
	
	Identifier = NAME_None;
}

bool AStructure::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);

	return AChunkManager::IsActorNetRelevent(this, ViewTarget);
}

FName AStructure::GetIdentifier() const
{
	return Identifier;
}