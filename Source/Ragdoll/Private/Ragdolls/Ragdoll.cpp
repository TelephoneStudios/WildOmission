// Copyright Telephone Studios. All Rights Reserved.


#include "Ragdolls/Ragdoll.h"
#include "Components/TimerDespawnComponent.h"
#include "Components/HarvestableComponent.h"
#include "GameFramework/PhysicsVolume.h"
#include "Actors/Chunk.h"
#include "ChunkManager.h"
#include "Net/UnrealNetwork.h"

ARagdoll::ARagdoll()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	SetNetUpdateFrequency(2.0f);
	
	RagdollRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RagdollRootComponent"));
	RootComponent = RagdollRootComponent;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_PhysicsBody, ECollisionResponse::ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Destructible, ECollisionResponse::ECR_Block);
	MeshComponent->SetShouldUpdatePhysicsVolume(true);
	MeshComponent->SetupAttachment(RagdollRootComponent);

	DespawnComponent = CreateDefaultSubobject<UTimerDespawnComponent>(TEXT("DespawnComponent"));
	DespawnComponent->SetDespawnTime(600.0f);

	HarvestableComponent = CreateDefaultSubobject<UHarvestableComponent>(TEXT("HarvestableComponent"));
}

void ARagdoll::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority())
	{
		return;
	}

	HandleInWater();
}

FName ARagdoll::GetIdentifier() const
{
	return Identifier;
}

USkeletalMeshComponent* ARagdoll::GetMeshComponent() const
{
	return MeshComponent;
}

void ARagdoll::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	UpdateAttachedChunk();
}

void ARagdoll::HandleInWater()
{
	if (MeshComponent == nullptr)
	{
		return;
	}

	APhysicsVolume* MeshPhysicsVolume = MeshComponent->GetPhysicsVolume();
	if (MeshPhysicsVolume == nullptr)
	{
		return;
	}

	if (MeshPhysicsVolume->bWaterVolume == false)
	{
		return;
	}

	const FVector ForceVector = FVector::UpVector * 1000.0f;
	MeshComponent->AddImpulse(ForceVector);
}

void ARagdoll::UpdateAttachedChunk()
{
	AChunkManager* ChunkManager = AChunkManager::GetChunkManager();
	if (ChunkManager == nullptr)
	{
		return;
	}

	const float ChunkSize = AChunk::GetVertexSize() * AChunk::GetVertexDistanceScale();
	const FVector ThisLocation(this->GetActorLocation());
	const FIntVector2 ThisChunkLocation(ThisLocation.X / ChunkSize, ThisLocation.Y / ChunkSize);

	FSpawnedChunk SpawnedChunk;
	ChunkManager->GetSpawnedChunk(ThisChunkLocation, SpawnedChunk);
	if (SpawnedChunk.Chunk == nullptr)
	{
		return;
	}

	this->AttachToActor(SpawnedChunk.Chunk, FAttachmentTransformRules::KeepWorldTransform);
}
