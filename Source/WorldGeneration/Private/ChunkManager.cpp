// Copyright Telephone Studios. All Rights Reserved.


#include "ChunkManager.h"
#include "Actors/Chunk.h"
#include "Components/ChunkInvokerComponent.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Curves/CurveFloat.h"
#include "UObject/ConstructorHelpers.h"
#include "Log.h"

static AChunkManager* Instance = nullptr;
static UCurveFloat* FlattenHeightCurve = nullptr;
static UDataTable* BiomeGenerationDataTable = nullptr;

// Sets default values
AChunkManager::AChunkManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;

	static ConstructorHelpers::FObjectFinder<UCurveFloat> FlattenHeightCurveBlueprint(TEXT("/Game/WorldGeneration/Curves/Curve_FlattenCurve"));
	if (FlattenHeightCurveBlueprint.Succeeded())
	{
		FlattenHeightCurve = FlattenHeightCurveBlueprint.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<UDataTable> BiomeDataTableBlueprint(TEXT("/Game/WorldGeneration/DataTables/DT_BiomeGenerationData"));
	if (BiomeDataTableBlueprint.Succeeded())
	{
		BiomeGenerationDataTable = BiomeDataTableBlueprint.Object;
	}

	static ConstructorHelpers::FClassFinder<AChunk> ChunkBlueprint(TEXT("/Game/WorldGeneration/BP_Chunk"));
	if (ChunkBlueprint.Succeeded())
	{
		ChunkClass = ChunkBlueprint.Class;
	}
}

void AChunkManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority())
	{
		return;
	}

	TArray<UChunkInvokerComponent*> ChunkInvokers = UChunkInvokerComponent::GetAllInvokers();

	RemoveOutOfRangeChunks(ChunkInvokers);
	SpawnInRangeChunks(ChunkInvokers);
	
}

void AChunkManager::RemoveOutOfRangeChunks(const TArray<UChunkInvokerComponent*>& ChunkInvokers)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	TArray<FSpawnedChunk> NewSpawnedChunks = SpawnedChunks;
	for (const FSpawnedChunk& SpawnedChunk : SpawnedChunks)
	{
		if (!IsValid(SpawnedChunk.Chunk))
		{
			continue;
		}

		const FVector ChunkWorldLocation = SpawnedChunk.Chunk->GetActorLocation();

		bool InRangeOfInvoker = false;
		for (UChunkInvokerComponent* ChunkInvoker : ChunkInvokers)
		{
			if (ChunkInvoker == nullptr)
			{
				continue;
			}

			const FVector FlattenedInvokerLocation(ChunkInvoker->GetComponentLocation().X, ChunkInvoker->GetComponentLocation().Y, 0.0f);
			if (AChunk::FastDistance(FlattenedInvokerLocation, ChunkWorldLocation) >= (ChunkInvoker->GetRenderDistanceCentimeters()))
			{
				continue;
			}

			InRangeOfInvoker = true;
			break;
		}

		if (InRangeOfInvoker)
		{
			continue;
		}

		NewSpawnedChunks.Remove(SpawnedChunk);

		SaveChunkData(SpawnedChunk.Chunk, true);
	}

	SpawnedChunks = NewSpawnedChunks;
}

void AChunkManager::SpawnInRangeChunks(const TArray<UChunkInvokerComponent*>& ChunkInvokers)
{
	for (UChunkInvokerComponent* ChunkInvoker : ChunkInvokers)
	{
		if (ChunkInvoker == nullptr)
		{
			continue;
		}

		SpawnChunksAtLocation(ChunkInvoker->GetComponentLocation(), ChunkInvoker->GetRenderDistance());
	}
}

void AChunkManager::SpawnChunksAtLocation(const FVector& Location, const uint8& RenderDistance)
{
	const float ChunkSize = AChunk::GetVertexSize() * AChunk::GetVertexDistanceScale();
	const FVector FlattenedLocation(Location.X, Location.Y, 0.0f);
	const FIntVector2 ChunkLocation(Location.X / ChunkSize,
		Location.Y / ChunkSize);

	// Generate/load new in range chunks
	for (int32 RenderX = -RenderDistance; RenderX <= RenderDistance; ++RenderX)
	{
		for (int32 RenderY = -RenderDistance; RenderY <= RenderDistance; ++RenderY)
		{
			FSpawnedChunk SpawnedChunk;
			SpawnedChunk.GridLocation = FIntVector2(RenderX, RenderY) + ChunkLocation;

			const FVector ChunkWorldLocation(SpawnedChunk.GridLocation.X * ChunkSize,
				SpawnedChunk.GridLocation.Y * ChunkSize, 0.0f);

			if (AChunk::FastDistance(FlattenedLocation, ChunkWorldLocation) >= (RenderDistance * ChunkSize))
			{
				continue;
			}

			if (!SpawnedChunks.Contains(SpawnedChunk))
			{
				SpawnChunk(SpawnedChunk);

				if (!IsValid(SpawnedChunk.Chunk))
				{
					continue;
				}

				SpawnedChunks.Add(SpawnedChunk);

				// Check if existing chuck, then load
				// otherwise generate.
				FChunkData ChunkSaveData;
				ChunkSaveData.GridLocation = SpawnedChunk.GridLocation;
				const int32 SaveIndex = ChunkData.Find(ChunkSaveData);
				if (SaveIndex != INDEX_NONE)
				{
					LoadChunk(SpawnedChunk, ChunkData[SaveIndex]);
				}
				else
				{
					GenerateChunk(SpawnedChunk);
				}

			}
		}
	}
}

void AChunkManager::SetChunkData(const TArray<FChunkData> InChunkData)
{
	ChunkData = InChunkData;
}

void AChunkManager::SaveAllSpawnedChunks()
{
	for (const FSpawnedChunk& SpawnedChunk : SpawnedChunks)
	{
		SaveChunkData(SpawnedChunk.Chunk);
	}
}

void AChunkManager::SaveChunkData(AChunk* ChunkToSave, bool AlsoDestroy)
{
	if (ChunkToSave == nullptr)
	{
		UE_LOG(LogWorldGeneration, Warning, TEXT("Failed to save chunk, chunk was nullptr."));
		return;
	}

	FChunkData ChunkSaveData;
	ChunkToSave->Save(ChunkSaveData, AlsoDestroy);

	const int32 ChunkIndex = ChunkData.Find(ChunkSaveData);
	if (ChunkIndex == INDEX_NONE)
	{
		ChunkData.Add(ChunkSaveData);
		return;
	}

	if (!ChunkData.IsValidIndex(ChunkIndex))
	{
		return;
	}

	ChunkData[ChunkIndex] = ChunkSaveData;
}

TArray<FChunkData> AChunkManager::GetAllChunkData() const
{
	return ChunkData;
}

bool AChunkManager::GetChunkData(const FIntVector2& ChunkLocation, FChunkData& OutChunkData) const
{
	FChunkData ChunkDataLocator;
	ChunkDataLocator.GridLocation = ChunkLocation;

	const int32 ChunkIndex = ChunkData.Find(ChunkDataLocator);
	if (ChunkIndex == INDEX_NONE)
	{
		return false;
	}

	OutChunkData = ChunkData[ChunkIndex];
	return true;
}

TArray<FSpawnedChunk> AChunkManager::GetAllSpawnedChunks() const
{
	return SpawnedChunks;
}

bool AChunkManager::GetSpawnedChunk(const FIntVector2& ChunkLocation, FSpawnedChunk& OutSpawnedChunk) const
{
	FSpawnedChunk SpawnedChunkLocator;
	SpawnedChunkLocator.GridLocation = ChunkLocation;

	const int32 ChunkIndex = SpawnedChunks.Find(SpawnedChunkLocator);
	if (ChunkIndex == INDEX_NONE)
	{
		return false;
	}

	OutSpawnedChunk = SpawnedChunks[ChunkIndex];
	return true;
}

void AChunkManager::AddSpawnedChunk(const FSpawnedChunk& InSpawnedChunk)
{
	if (!IsValid(InSpawnedChunk.Chunk))
	{
		return;
	}

	SpawnedChunks.Add(InSpawnedChunk);
}

void AChunkManager::RemoveSpawnedChunk(const FSpawnedChunk& InSpawnedChunk)
{
	SpawnedChunks.Remove(InSpawnedChunk);
}

AChunk* AChunkManager::GetChunkAtLocation(const FIntVector2& ChunkLocation)
{
	FSpawnedChunk SpawnedChunk;
	SpawnedChunk.GridLocation = ChunkLocation;

	const int32 SpawnedIndex = SpawnedChunks.Find(SpawnedChunk);
	if (SpawnedIndex == INDEX_NONE)
	{
		SpawnChunk(SpawnedChunk);

		if (!IsValid(SpawnedChunk.Chunk))
		{
			return nullptr;
		}

		SpawnedChunks.Add(SpawnedChunk);

		// Check if we have existing data to load
		FChunkData ChunkSaveData;
		ChunkSaveData.GridLocation = SpawnedChunk.GridLocation;
		const int32 SaveIndex = ChunkData.Find(ChunkSaveData);
		if (SaveIndex != INDEX_NONE)
		{
			LoadChunk(SpawnedChunk, ChunkData[SaveIndex]);
		}
		else
		{
			GenerateChunk(SpawnedChunk);
		}

		return SpawnedChunk.Chunk;
	}

	return SpawnedChunks[SpawnedIndex].Chunk;
}

AChunk* AChunkManager::GetChunkAtLocation(const FVector& ChunkLocation)
{
	const float ChunkSize = AChunk::GetVertexSize() * AChunk::GetVertexDistanceScale();
	const FIntVector2 ChunkGridLocation(FMath::RoundToInt32(ChunkLocation.X / ChunkSize), FMath::RoundToInt32(ChunkLocation.Y / ChunkSize));
	return GetChunkAtLocation(ChunkGridLocation);
}

void AChunkManager::HandleActorRenderDistanceVisibility(UWorld* WorldContextObject, AActor* InActor)
{
	if (Instance == nullptr || WorldContextObject == nullptr || WorldContextObject->IsEditorWorld() || InActor == nullptr)
	{
		return;
	}

	const FVector ActorLocation = InActor->GetActorLocation();
	const float ChunkSize = AChunk::GetVertexSize() * AChunk::GetVertexDistanceScale();
	
	AChunk* ActorChunk = Instance->GetChunkAtLocation(FIntVector2(FMath::RoundToInt32(ActorLocation.X / ChunkSize), FMath::RoundToInt32(ActorLocation.Y / ChunkSize)));
	if (ActorChunk == nullptr)
	{
		return;
	}
	
	UStaticMeshComponent* ActorMeshComponent = InActor->FindComponentByClass<UStaticMeshComponent>();
	if (ActorMeshComponent == nullptr)
	{
		return;
	}

	if (ActorChunk->IsChunkHidden() == false)
	{
		return;
	}

	ActorMeshComponent->SetVisibility(false);
}

bool AChunkManager::IsActorNetRelevent(const AActor* ActorToTest, const AActor* ViewingActor)
{
	UChunkInvokerComponent* ChunkInvoker = ViewingActor->FindComponentByClass<UChunkInvokerComponent>();
	if (ChunkInvoker == nullptr)
	{
		return false;
	}

	const FVector CorrectedSrcLocation(ViewingActor->GetActorLocation().X, ViewingActor->GetActorLocation().Y, 0.0f);
	const FVector CorrectedThisLocation(ActorToTest->GetActorLocation().X, ActorToTest->GetActorLocation().Y, 0.0f);
	const float Distance = FVector::Distance(CorrectedSrcLocation, CorrectedThisLocation);

	return Distance < ChunkInvoker->GetRenderDistanceCentimeters();
}

void AChunkManager::ClearDecorationsAroundChunk(const FIntVector2& Origin, const FIntVector2& Size)
{
	Async(EAsyncExecution::ThreadPool, [this, Origin, Size]()
		{
			for (int32 X = Origin.X - Size.X; X <= Origin.X + Size.X; ++X)
			{
				for (int32 Y = Origin.Y - Size.Y; Y <= Origin.Y + Size.Y; ++Y)
				{
					FSpawnedChunk SpawnedChunk;
					SpawnedChunk.GridLocation = FIntVector2(X, Y);
					const int32 SpawnedChunkIndex = SpawnedChunks.Find(SpawnedChunk);
					if (SpawnedChunkIndex == -1)
					{
						continue;
					}

					AsyncTask(ENamedThreads::GameThread, [this, SpawnedChunkIndex]() 
						{
							SpawnedChunks[SpawnedChunkIndex].Chunk->ClearAllAttachedActors();
						});
					
				}
			}
		});
	
}

void AChunkManager::FlattenTerrainAroundChunk(const FIntVector2& Origin, const FIntVector2& Size, float DesiredHeight)
{
	Async(EAsyncExecution::ThreadPool, [this, Origin, Size, DesiredHeight]()
		{
			const int32 ChunkStartX = Origin.X - Size.X;
			const int32 ChunkEndX = Origin.X + Size.X;
			const int32 ChunkStartY = Origin.Y - Size.Y;
			const int32 ChunkEndY = Origin.Y + Size.Y;

			const int32  ChunkVertexSize = AChunk::GetVertexSize();
			const int32 HalfVertexSize = ChunkVertexSize * 0.5f;

			const FVector2D ChunkOriginVertexSpace(Origin.X * ChunkVertexSize, Origin.Y * ChunkVertexSize);
			const FVector2D ChunkStartVertexSpace(Origin.X * ChunkVertexSize, ChunkStartY * ChunkVertexSize);
			const float MaxDistance = FVector2D::Distance(ChunkOriginVertexSpace, ChunkStartVertexSpace);

			for (int32 ChunkX = ChunkStartX; ChunkX <= ChunkEndX; ++ChunkX)
			{
				for (int32 ChunkY = ChunkStartY; ChunkY <= ChunkEndY; ++ChunkY)
				{
					FSpawnedChunk SpawnedChunk;
					SpawnedChunk.GridLocation = FIntVector2(ChunkX, ChunkY);
					const int32 SpawnedChunkIndex = SpawnedChunks.Find(SpawnedChunk);
					if (SpawnedChunkIndex == -1)
					{
						continue;
					}

					const FVector2D ChunkVertexSpace(ChunkX * ChunkVertexSize, ChunkY * ChunkVertexSize);

					TArray<float> ChunkHeightData = SpawnedChunks[SpawnedChunkIndex].Chunk->GetHeightData();
					for (int32 HeightX = 0; HeightX <= AChunk::GetVertexSize(); ++HeightX)
					{
						for (int32 HeightY = 0; HeightY <= AChunk::GetVertexSize(); ++HeightY)
						{
							const int32 HeightDataIndex = (HeightX * (AChunk::GetVertexSize() + 1)) + HeightY;
							if (!ChunkHeightData.IsValidIndex(HeightDataIndex))
							{
								continue;
							}

							const FVector2D CurrentVertexSpace(HeightX + ChunkVertexSpace.X, HeightY + ChunkVertexSpace.Y);

							const float CurrentDistance = FVector2D::Distance(ChunkOriginVertexSpace, CurrentVertexSpace);
							const float Alpha = FlattenHeightCurve->GetFloatValue(FMath::Clamp((CurrentDistance / MaxDistance), 0.0f, 1.0f));
							ChunkHeightData[HeightDataIndex] = FMath::Lerp(DesiredHeight, ChunkHeightData[HeightDataIndex], Alpha);
						}
					}

					AsyncTask(ENamedThreads::GameThread, [this, Origin, Size, ChunkX, ChunkY, ChunkHeightData, SpawnedChunkIndex]()
						{
							SpawnedChunks[SpawnedChunkIndex].Chunk->SetHeightData(ChunkHeightData);
							SpawnedChunks[SpawnedChunkIndex].Chunk->OnLoadedTerrainData();
							if (ChunkX > (Origin.X + (Size.X * 0.5f))
								|| ChunkX < (Origin.X - (Size.X * 0.5f))
								|| ChunkY >(Origin.Y + (Size.Y * 0.5f))
								|| ChunkY < (Origin.Y - (Size.Y * 0.5f)))
							{
								SpawnedChunks[SpawnedChunkIndex].Chunk->Redecorate();
							}
						});
				}
			}
		});
}

FVector AChunkManager::GetWorldSpawnPoint()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return FVector::ZeroVector;
	}

	const float ChunkSize = AChunk::GetVertexSize() * AChunk::GetVertexDistanceScale();

	const int32 TestRadius = 128;
	FIntVector2 ValidChunk(0, 0);
	if (PlayerSpawnChunk.IsSet == false)
	{
		for (int32 X = -TestRadius; X < TestRadius; ++X)
		{
			for (int32 Y = -TestRadius; Y < TestRadius; ++Y)
			{
				const FVector2D TestLocation(
					X * ChunkSize,
					Y * ChunkSize
				);
				const float Continentalness = AChunk::GetContinentalnessAtLocation(TestLocation, true);
				if (Continentalness > 0.0f || Continentalness < -0.05f)
				{
					// If not pick a new starting point and keep going
					continue;
				}

				ValidChunk = FIntVector2(X, Y);
				break;
			}
		}

		PlayerSpawnChunk.ChunkLocation = ValidChunk;
		PlayerSpawnChunk.IsSet = true;
	}
	else
	{
		ValidChunk = PlayerSpawnChunk.ChunkLocation;
	}
	
	// Line trace down to find ground
	FHitResult HitResult;

	// Ensure the chunk we want to start at exists
	(void*)GetChunkAtLocation(FIntVector2(ValidChunk.X, ValidChunk.Y));
	const FVector ValidChunkWorldLocation((ValidChunk.X * ChunkSize) - (ChunkSize * 0.5f), (ValidChunk.Y * ChunkSize) - (ChunkSize * 0.5f), 0.0f);

	// Loop through all points on chunk to see if there is a good spot
	for (int32 X = 0; X <= AChunk::GetVertexSize(); ++X)
	{
		for (int32 Y = 0; Y <= AChunk::GetVertexSize(); ++Y)
		{
			const FVector Start((X * AChunk::GetVertexDistanceScale()) + ValidChunkWorldLocation.X, (Y * AChunk::GetVertexDistanceScale()) + ValidChunkWorldLocation.Y, AChunk::GetMaxHeight());
			const FVector End(Start.X, Start.Y, -AChunk::GetMaxHeight());
			if (!World->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility))
			{
				continue;
			}

			UPrimitiveComponent* HitComponent = HitResult.GetComponent();
			if (HitComponent == nullptr || !HitComponent->ComponentHasTag(TEXT("Ground")))
			{
				continue;
			}

			break;
		}
	}

	return HitResult.ImpactPoint;
}

FPlayerSpawnChunk AChunkManager::GetPlayerSpawnChunk() const
{
	return PlayerSpawnChunk;
}

void AChunkManager::SetPlayerSpawnChunk(const FPlayerSpawnChunk& InPlayerSpawnChunk)
{
	this->PlayerSpawnChunk = InPlayerSpawnChunk;
}

uint8 AChunkManager::GetSurfaceTypeAtLocation(const FVector& TestLocation) const
{
	const int32 ChunkSize = AChunk::GetVertexSize() * AChunk::GetVertexDistanceScale();
	const FIntVector2 TestChunkLocation(
		FMath::RoundToInt32(TestLocation.X / ChunkSize),
		FMath::RoundToInt32(TestLocation.Y / ChunkSize));

	FSpawnedChunk SpawnedChunk;
	SpawnedChunk.GridLocation = TestChunkLocation;
	const int32 SpawnedChunkIndex = SpawnedChunks.Find(SpawnedChunk);
	if (SpawnedChunkIndex == INDEX_NONE)
	{
		return 0;
	}
	
	const AChunk* TestChunk = SpawnedChunks[SpawnedChunkIndex].Chunk;
	if (TestChunk == nullptr)
	{
		return 0;
	}

	// Get surface index at point at which we are testing for within this chunk
	const float ChunkHalfSize = ChunkSize * 0.5f;
	const float AX = ((TestLocation.X - ChunkHalfSize) / ChunkSize);
	const float AY = ((TestLocation.Y - ChunkHalfSize) / ChunkSize);
	const int32 TestX = (AX - FMath::Floor(AX)) * AChunk::GetVertexSize();
	const int32 TestY = (AY - FMath::Floor(AY)) * AChunk::GetVertexSize();

	const int32 ArrayIndex = (TestX * (AChunk::GetVertexSize() + 1)) + TestY;

	const TArray<uint8> SurfaceData = TestChunk->GetSurfaceData();
	if (!SurfaceData.IsValidIndex(ArrayIndex))
	{
		return 0;
	}

	return SurfaceData[ArrayIndex];
}

FBiomeGenerationData* AChunkManager::GetBiomeAtLocation(const FVector& TestLocation)
{
	return AChunk::GetBiomeAtLocation(FVector2D(TestLocation.X, TestLocation.Y));
}

void AChunkManager::SetGenerationSeed(const uint32& Seed)
{
	AChunk::SetGenerationSeed(Seed);
}

uint32 AChunkManager::GetGenerationSeed()
{
	return AChunk::GetGenerationSeed();
}

// Called when the game starts or when spawned
void AChunkManager::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (World == nullptr || IsValid(Instance))
	{
		return;
	}

	SetChunkManager(this);
}

void AChunkManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	Instance = nullptr;
}

void AChunkManager::SpawnChunk(FSpawnedChunk& OutSpawnedChunk) const
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// TODO this could be simplified
	const FVector SpawnLocation(OutSpawnedChunk.GridLocation.X * AChunk::GetVertexSize() * AChunk::GetVertexDistanceScale(),
		OutSpawnedChunk.GridLocation.Y * AChunk::GetVertexSize() * AChunk::GetVertexDistanceScale(), 0.0f);
	OutSpawnedChunk.Chunk = World->SpawnActor<AChunk>(ChunkClass, SpawnLocation, FRotator::ZeroRotator);

	//OutSpawnedChunk.Chunk->SetChunkLocation(FIntVector2(OutSpawnedChunk.GridLocation.X, OutSpawnedChunk.GridLocation.Y));
}

void AChunkManager::GenerateChunk(const FSpawnedChunk& InSpawnedChunk) const
{
	// Ensure we are prossesing valid chunk
	if (!IsValid(InSpawnedChunk.Chunk))
	{
		return;
	}

	// We get neighbors from ChunkData instead of SpawnedChunks
	// because they might not be spawned when we are generating this chunk

	// Neighbors
	TArray<FChunkData> NeighborsData{
		// Top
		InSpawnedChunk.GridLocation + FIntVector2(0, 1),
		// Bottom
		InSpawnedChunk.GridLocation + FIntVector2(0, -1),
		// Left
		InSpawnedChunk.GridLocation + FIntVector2(-1, 0),
		// Right
		InSpawnedChunk.GridLocation + FIntVector2(1, 0),
		// Top Left
		InSpawnedChunk.GridLocation + FIntVector2(-1, 1),
		// Top Right
		InSpawnedChunk.GridLocation + FIntVector2(1, 1),
		// Bottom Left
		InSpawnedChunk.GridLocation + FIntVector2(-1, -1),
		// Bottom Right
		InSpawnedChunk.GridLocation + FIntVector2(1, -1)
	};
	
	// Populate NeighborData
	for (FChunkData& NeighborData : NeighborsData)
	{
		const int32 NeighborIndex = ChunkData.Find(NeighborData);

		if (NeighborIndex == INDEX_NONE)
		{
			continue;
		}

		NeighborData = ChunkData[NeighborIndex];
	}

	// Pass neighbors to new chunk for generation
	InSpawnedChunk.Chunk->Generate(NeighborsData);
}

void AChunkManager::LoadChunk(const FSpawnedChunk& InSpawnedChunk, const FChunkData& InChunkData) const
{
	if (!IsValid(InSpawnedChunk.Chunk))
	{
		return;
	}

	InSpawnedChunk.Chunk->Load(InChunkData);
}

TArray<FBiomeGenerationData*> AChunkManager::GetAllPossibleBiomes()
{
	TArray<FBiomeGenerationData*> Biomes;
	if (BiomeGenerationDataTable == nullptr)
	{
		return Biomes;
	}

	static const FString ContextString(TEXT("All Biomes Data Context"));
	
	BiomeGenerationDataTable->GetAllRows<FBiomeGenerationData>(ContextString, Biomes);

	return Biomes;
}

FBiomeGenerationData* AChunkManager::GetBiomeGenerationData(const FName& BiomeName)
{
	if (BiomeGenerationDataTable == nullptr)
	{
		return nullptr;
	}

	static const FString ContextString(TEXT("Biome Generation Data Context"));

	return BiomeGenerationDataTable->FindRow<FBiomeGenerationData>(BiomeName, ContextString, true);
}

void AChunkManager::SetChunkManager(AChunkManager* NewInstance)
{
	Instance = NewInstance;
}

AChunkManager* AChunkManager::GetChunkManager()
{
	return Instance;
}