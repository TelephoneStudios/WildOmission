// Copyright Telephone Studios. All Rights Reserved.


#include "Structures/LootSpawnComponent.h"
#include "Deployables/Deployable.h"

// Sets default values for this component's properties
ULootSpawnComponent::ULootSpawnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	MinSpawnFrequencySeconds = 120.0f;
	MaxSpawnFrequencySeconds = 600.0f;
	UseRandomYawRotation = true;
	TimeTillNextSpawnSeconds = 0.0f;

	CurrentLoot = nullptr;
}


// Called when the game starts
void ULootSpawnComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void ULootSpawnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr || !OwnerActor->HasAuthority())
	{
		return;
	}

	TimeTillNextSpawnSeconds -= DeltaTime;

	if (TimeTillNextSpawnSeconds > 0.0f)
	{
		return;
	}

	if (IsValid(CurrentLoot))
	{
		CurrentLoot->Destroy();
		CurrentLoot = nullptr;
	}
	
	TimeTillNextSpawnSeconds = FMath::RandRange(MinSpawnFrequencySeconds, MaxSpawnFrequencySeconds);

	UWorld* World = GetWorld();
	const int32 LootIndex = FMath::RandRange(0, LootToSpawn.Num() - 1);
	if (World == nullptr || !LootToSpawn.IsValidIndex(LootIndex) || LootToSpawn[LootIndex] == nullptr)
	{
		return;
	}
	
	//https://forums.somethingawful.com/showthread.php?threadid=3817946
	const FRotator SpawnRotation = UseRandomYawRotation ? FRotator(0.0, FMath::RandRange(0.0f, 360.0f), 0.0f) : this->GetComponentRotation();

	CurrentLoot = World->SpawnActor<ADeployable>(LootToSpawn[LootIndex], this->GetComponentLocation(), SpawnRotation);
	CurrentLoot->OnSpawn();

	TArray<AActor*> OverlappingActors;
	CurrentLoot->GetOverlappingActors(OverlappingActors);
	for (AActor* OverlappingActor : OverlappingActors)
	{
		if (!IsValid(OverlappingActor))
		{
			continue;
		}

		ADeployable* OverlappingDeployable = Cast<ADeployable>(OverlappingActor);
		if (!IsValid(OverlappingDeployable))
		{
			continue;
		}

		CurrentLoot->Destroy();
		CurrentLoot = OverlappingDeployable;
		break;
	}
}
