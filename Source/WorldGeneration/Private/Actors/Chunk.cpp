// Copyright Telephone Studios. All Rights Reserved.


#include "Actors/Chunk.h"
#include "ProceduralMeshComponent.h"
#include "Components/ChunkSaveComponent.h"
#include "ChunkManager.h"
#include "Components/ChunkInvokerComponent.h"
#include "Actors/ChunkInvokerActor.h"
#include "Structs/SpawnQuery.h"
#include "Curves/CurveFloat.h"
#include "Noise/PerlinNoise.hpp"
#include "Kismet/KismetMathLibrary.h"
#include "KismetProceduralMeshLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Structures/Structure.h"
#include "UObject/ConstructorHelpers.h"

static siv::PerlinNoise Noise(10);
static uint32 Seed = 0;
const static int32 VERTEX_SIZE = 16;
const static float VERTEX_DISTANCE_SCALE = 100.0f;
const static float CHUNK_SIZE_CENTIMETERS = VERTEX_SIZE * VERTEX_DISTANCE_SCALE;
const static float UV_SCALE = 0.0625f;
const static float MAX_HEIGHT = 1000000.0f;

static UCurveFloat* ContinentalnessHeightCurve = nullptr;;
static UCurveFloat* ErosionHeightCurve = nullptr;
static UCurveFloat* PeaksAndValleysHeightCurve = nullptr;
static UCurveFloat* StructureFactorCurve = nullptr;

static UMaterialInterface* ChunkMaterial = nullptr;

const static FColor STONE_VERTEX_COLOR = FColor(255, 0, 0);
const static FColor GRASS_VERTEX_COLOR = FColor(0, 255, 0);
const static FColor DIRT_VERTEX_COLOR = FColor(0, 0, 255);
const static FColor SAND_VERTEX_COLOR = FColor(255, 255, 0);
const static FColor GRAVEL_VERTEX_COLOR = FColor(0, 255, 255);
const static FColor SNOW_VERTEX_COLOR = FColor(255, 255, 255);
const static FColor RESERVED_VERTEX_COLOR = FColor(0, 0, 0);

// Sets default values
AChunk::AChunk()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	NetUpdateFrequency = 2.0f;
	NetDormancy = ENetDormancy::DORM_DormantAll;
	
	MeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->ComponentTags.Add(TEXT("Ground"));
	RootComponent = MeshComponent;

	WaterMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterMeshComponent"));
	WaterMeshComponent->SetMobility(EComponentMobility::Stationary);
	WaterMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WaterMeshComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	WaterMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	WaterMeshComponent->ComponentTags.Add(TEXT("Water"));
	WaterMeshComponent->SetupAttachment(MeshComponent);

	SaveComponent = CreateDefaultSubobject<UChunkSaveComponent>(TEXT("SaveComponent"));

	ChunkHidden = false;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> TerrainMaterial(TEXT("/Game/WorldGeneration/M_Terrain"));
	if (TerrainMaterial.Succeeded())
	{
		ChunkMaterial = TerrainMaterial.Object;
	}

	static ConstructorHelpers::FObjectFinder<UCurveFloat> ContinentalnessHeightCurveBlueprint(TEXT("/Game/WorldGeneration/Curves/Curve_Continentalness_Height"));
	if (ContinentalnessHeightCurveBlueprint.Succeeded())
	{
		ContinentalnessHeightCurve = ContinentalnessHeightCurveBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UCurveFloat> ErosionHeightCurveBlueprint(TEXT("/Game/WorldGeneration/Curves/Curve_Erosion_Height"));
	if (ErosionHeightCurveBlueprint.Succeeded())
	{
		ErosionHeightCurve = ErosionHeightCurveBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UCurveFloat> PeaksAndValleysHeightCurveBlueprint(TEXT("/Game/WorldGeneration/Curves/Curve_PeaksAndValleys_Height"));
	if (PeaksAndValleysHeightCurveBlueprint.Succeeded())
	{
		PeaksAndValleysHeightCurve = PeaksAndValleysHeightCurveBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UCurveFloat> StructureFactorCurveBlueprint(TEXT("/Game/WorldGeneration/Curves/Curve_StructureFactor"));
	if (StructureFactorCurveBlueprint.Succeeded())
	{
		StructureFactorCurve = StructureFactorCurveBlueprint.Object;
	}
}

// Called when the game starts or when spawned
void AChunk::BeginPlay()
{
	Super::BeginPlay();

	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &AChunk::UpdateVisibility);
	GetWorldTimerManager().SetTimer(UpdateVisibilityTimerHandle, TimerDelegate, 1.0f, true);

	// So I'm not actually so sure what purpose the code below serves if its being ignored by the server
	/*if (HasAuthority())
	{
		return;
	}

	AChunkManager* ChunkManager = AChunkManager::GetChunkManager();
	if (ChunkManager == nullptr)
	{
		return;
	}

	FSpawnedChunk SpawnedChunk;
	SpawnedChunk.GridLocation = GetChunkLocation();
	SpawnedChunk.Chunk = this;

	ChunkLoc = GetChunkLocation();

	ChunkManager->AddSpawnedChunk(SpawnedChunk);*/
}

void AChunk::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	/*if (HasAuthority())
	{
		return;
	}

	AChunkManager* ChunkManager = AChunkManager::GetChunkManager();
	if (ChunkManager == nullptr)
	{
		return;
	}

	FSpawnedChunk SpawnedChunk;
	SpawnedChunk.GridLocation = this->GetChunkLocation();
	SpawnedChunk.Chunk = this;

	ChunkManager->RemoveSpawnedChunk(SpawnedChunk);*/
}

bool AChunk::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);

	return AChunkManager::IsActorNetRelevent(this, ViewTarget);
}

void AChunk::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AChunk, HeightData, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AChunk, SurfaceData, COND_InitialOnly);
}

void AChunk::Generate(const TArray<FChunkData>& Neighbors)
{
	GenerateTerrain(Neighbors);
	GenerateFeatures();
}

void AChunk::SetChunkHidden(bool Hidden)
{
	const bool AlreadyHidden = Hidden && ChunkHidden;
	const bool AlreadyShown = !Hidden && !ChunkHidden;

	if (AlreadyHidden || AlreadyShown)
	{
		return;
	}

	if (MeshComponent)
	{
		MeshComponent->SetVisibility(!Hidden);
	}
	if (WaterMeshComponent)
	{
		WaterMeshComponent->SetVisibility(!Hidden);
	}

	TArray<AActor*> AttachedActors;
	this->GetAttachedActors(AttachedActors);

	for (AActor* AttachedActor : AttachedActors)
	{
		if (AttachedActor == nullptr)
		{
			continue;
		}
	
		UStaticMeshComponent* AttachedActorMeshComponent = AttachedActor->FindComponentByClass<UStaticMeshComponent>();
		if (AttachedActorMeshComponent == nullptr)
		{
			continue;
		}

		AttachedActorMeshComponent->SetVisibility(!Hidden, false);
	}

	ChunkHidden = Hidden;
}

void AChunk::OnLoadedTerrainData()
{
	if (HeightData.IsEmpty() || SurfaceData.IsEmpty())
	{
		return;
	}

	CreateTerrainMesh();
}

void AChunk::Save(FChunkData& OutChunkData, bool AlsoDestroy)
{
	SaveComponent->Save(OutChunkData, AlsoDestroy);
}

void AChunk::Load(const FChunkData& InChunkData)
{
	SaveComponent->Load(InChunkData);
}

//***************************************************************************************
//	Noise
//***************************************************************************************
float AChunk::GetContinentalnessAtLocation(const FVector2D& Location, bool UseRawValue)
{
	if (ContinentalnessHeightCurve == nullptr)
	{
		return 0.0f;
	}

	const float ContinentalnessScale = 0.000005f;
	const float RawValue = Noise.octave2D(Location.X * ContinentalnessScale, Location.Y * ContinentalnessScale, 3);

	return UseRawValue ? RawValue : ContinentalnessHeightCurve->GetFloatValue(RawValue);
}

float AChunk::GetErosionAtLocation(const FVector2D& Location, bool UseRawValue)
{
	if (ErosionHeightCurve == nullptr)
	{
		return 0.0f;
	}

	const float ErosionScale = 0.00005f;
	const float ErosionOffset = 50000.0f;
	const FVector2D TestLocation = Location + ErosionOffset;

	const float RawValue = Noise.octave2D(TestLocation.X * ErosionScale, TestLocation.Y * ErosionScale, 3);

	return UseRawValue ? RawValue : ErosionHeightCurve->GetFloatValue(RawValue);
}

float AChunk::GetPeaksAndValleysAtLocation(const FVector2D& Location, bool UseRawValue)
{
	if (PeaksAndValleysHeightCurve == nullptr)
	{
		return 0.0f;
	}

	const float PeaksAndValleysScale = 0.00005f;
	const float PeaksAndValleysOffset = -50000.0f;
	const FVector2D TestLocation = Location + PeaksAndValleysOffset;
	const float RawValue = Noise.octave2D(TestLocation.X * PeaksAndValleysScale, TestLocation.Y * PeaksAndValleysScale, 3);
	return UseRawValue ? RawValue : PeaksAndValleysHeightCurve->GetFloatValue(RawValue);
}

float AChunk::GetStructureFactorAtLocation(const FVector2D& Location, bool UseRawValue)
{
	if (StructureFactorCurve == nullptr)
	{
		return 0.0f;
	}

	const float StructureFactorScale = 0.5f;
	const float StructureFactorOffset = -20000.0f;
	const FVector2D TestLocation = Location + StructureFactorOffset;
	const float RawValue = Noise.octave2D(TestLocation.X * StructureFactorScale, TestLocation.Y * StructureFactorScale, 3);
	return UseRawValue ? RawValue : StructureFactorCurve->GetFloatValue(RawValue);
}

//***************************************************************************************
//	Chunk Setters
//***************************************************************************************
void AChunk::SetGenerationSeed(const uint32& InSeed)
{
	Seed = InSeed;
	Noise.reseed(InSeed);
	FMath::RandInit(InSeed);
}

void AChunk::SetChunkLocation(const FIntVector2& InLocation)
{
	SetActorLocation(FVector(InLocation.X * CHUNK_SIZE_CENTIMETERS, InLocation.Y * CHUNK_SIZE_CENTIMETERS, 0.0f));
}

void AChunk::SetHeightData(const TArray<float>& InHeightData)
{
	HeightData = InHeightData;
}

void AChunk::SetSurfaceData(const TArray<uint8>& InSurfaceData)
{
	SurfaceData = InSurfaceData;
}

void AChunk::ClearAllAttachedDecorations()
{
	TArray<AActor*> AttachedActors;
	this->GetAttachedActors(AttachedActors);
	
	for (AActor* AttachedActor : AttachedActors)
	{
		if (Cast<AStructure>(AttachedActor) || AttachedActor == nullptr )
		{
			continue;
		}

		AttachedActor->Destroy();
	}
}

void AChunk::Redecorate()
{
	ClearAllAttachedDecorations();
	GenerateDecorations();
}

//***************************************************************************************
//	Chunk Getters
//***************************************************************************************
uint32 AChunk::GetGenerationSeed()
{
	return Seed;
}

FName AChunk::GetBiomeNameAtLocation(const FVector2D& Location)
{
	const float Contenentalness = GetContinentalnessAtLocation(Location, true);

	const float TempatureScale = 0.00001f;
	const float HumidityScale = 0.00005f;

	const float Tempature = (Noise.noise2D(Location.X * TempatureScale, Location.Y * TempatureScale) + 1.0f) * 0.5f;
	const float Humidity = (Noise.noise2D(Location.X * HumidityScale, Location.Y * TempatureScale) + 1.0f) * 0.5f;

	const float MinShore = -0.05f;
	const float MaxShore = 0.0f;

	const int32 TempatureIndex = FMath::RoundToInt32(Tempature / 0.25f);
	const int32 HumidityIndex = FMath::RoundToInt32(Humidity / 0.25f);

	// TODO implement other biomes like beaches and mountains later
	FName BiomeName = NAME_None;
	if (Contenentalness > MinShore && Contenentalness < MaxShore)
	{
		// Beach biome
		BiomeName = TEXT("Beach");

	}
	else if (Contenentalness >= MaxShore)
	{
		// Middle biome
		switch (TempatureIndex)
		{
		case 0:
			BiomeName = HumidityIndex < 2 ? TEXT("Snow_Plains") : TEXT("Snow_Forest");
			break;
		case 1:
			BiomeName = HumidityIndex < 2 ? TEXT("Snow_Plains") : TEXT("Snow_Forest");
			break;
		case 2:
			BiomeName = HumidityIndex < 2 ? TEXT("Plains") : TEXT("Forest");
			break;
		case 3:
			BiomeName = HumidityIndex < 2 ? TEXT("Desert") : TEXT("Birch_Forest");
			break;
		case 4:
			BiomeName = TEXT("Desert");
			break;
		default:
			BiomeName = TEXT("Plains");
			break;
		}
	}

	return BiomeName;
}

FBiomeGenerationData* AChunk::GetBiomeAtLocation(const FVector2D& Location)
{
	const FName BiomeName = GetBiomeNameAtLocation(Location);
	if (BiomeName == NAME_None)
	{
		return nullptr;
	}

	return AChunkManager::GetBiomeGenerationData(BiomeName);
}

void AChunk::GetTerrainDataAtLocation(const FVector2D& Location, float& OutHeight, uint8& OutSurface)
{
	const float RawContinentalness = GetContinentalnessAtLocation(Location, true);
	OutHeight = GetTerrainHeightAtLocation(Location);

	// 1 = Stone
	// 2 = Grass
	// 3 = Dirt
	// 4 = Sand
	// 5 = Gravel
	// 6 = Snow
	// 7 = Reserved

	OutSurface = 1;

	const FString BiomeName = GetBiomeNameAtLocation(Location).ToString().ToLower();
	TArray<float> TestPoints 
	{
		GetTerrainHeightAtLocation(Location + FVector2D(VERTEX_DISTANCE_SCALE, 0.0f)),
		GetTerrainHeightAtLocation(Location + FVector2D(-VERTEX_DISTANCE_SCALE, 0.0f)),
		GetTerrainHeightAtLocation(Location + FVector2D(0.0f, -VERTEX_DISTANCE_SCALE)),
		GetTerrainHeightAtLocation(Location + FVector2D(0.0f, VERTEX_DISTANCE_SCALE))
	};

	const float StoneSurfaceDifferenceThreshold = 100.0f;
	const float DirtSurfaceDifferenceThreshold = 50.0f;

	const bool ShouldBeStone = !IsWithinThreshold(TestPoints, OutHeight - StoneSurfaceDifferenceThreshold, OutHeight + StoneSurfaceDifferenceThreshold);
	const bool ShouldBeDirt = !IsWithinThreshold(TestPoints, OutHeight - DirtSurfaceDifferenceThreshold, OutHeight + DirtSurfaceDifferenceThreshold);

	// Ground Color
	if (BiomeName.StartsWith("Snow_"))
	{
		ShouldBeStone ? OutSurface = 1 : OutSurface = 6;
	}
	else if (BiomeName == TEXT("Desert") || RawContinentalness <= 0.0f)
	{
		// Sand
		OutSurface = 4;
	}
	else
	{	
		if (ShouldBeStone)
		{
			OutSurface = 1;
		}
		else if (ShouldBeDirt)
		{
			OutSurface = 3;
		}
		else
		{
			const bool HighAltitude = OutHeight > 10000.0f;
			// Snow or Grass
			HighAltitude ? OutSurface = 6 : OutSurface = 2;
		}
	}
}

float AChunk::GetTerrainHeightAtLocation(const FVector2D& Location)
{
	float Height = 0.0f;
	
	const float ContinentalnessInfluence = 1000.0f;
	const float ErosionInfluence = 5000.0f;
	const float PeaksAndValleysInfluence = 25000.0f;

	const float NormalizedContinentalnessHeight = GetContinentalnessAtLocation(Location);
	const float NormalizedErosionHeight = GetErosionAtLocation(Location);
	const float NormalizedPeaksAndValleysHeight = GetPeaksAndValleysAtLocation(Location);

	const float ContinentalnessHeight = NormalizedContinentalnessHeight * ContinentalnessInfluence;
	const float ErosionHeight = (NormalizedContinentalnessHeight + 1) * ErosionInfluence;
	const float PeaksAndValleysHeight = (NormalizedContinentalnessHeight + 1) * PeaksAndValleysInfluence;

	const float RawContinentalness = GetContinentalnessAtLocation(Location, true);
	const float RawErosion = GetErosionAtLocation(Location, true);
	const float RawPeaksAndValleys = GetPeaksAndValleysAtLocation(Location, true);

	// Height
	Height = ContinentalnessHeight;
	if (RawContinentalness > 0.0f)
	{
		Height += ErosionHeight * RawContinentalness;
		if (RawErosion > 0.0f)
		{
			Height += PeaksAndValleysHeight * RawErosion * RawContinentalness;
		}
	}

	return Height;
}

FIntVector2 AChunk::GetChunkLocation() const
{
	const FVector WorldLocation = GetActorLocation();
	return FIntVector2(FMath::RoundToInt32(WorldLocation.X / CHUNK_SIZE_CENTIMETERS), FMath::RoundToInt32(WorldLocation.Y / CHUNK_SIZE_CENTIMETERS));
}

bool AChunk::IsChunkHidden() const
{
	return ChunkHidden;
}

TArray<float> AChunk::GetHeightData() const
{
	return HeightData;
}

TArray<uint8> AChunk::GetSurfaceData() const
{
	return SurfaceData;
}

int32 AChunk::GetVertexSize()
{
	return VERTEX_SIZE;
}

float AChunk::GetVertexDistanceScale()
{
	return VERTEX_DISTANCE_SCALE;
}

float AChunk::GetMaxHeight()
{
	return MAX_HEIGHT;
}

//***************************************************************************************
//	Chunk Helpers
//***************************************************************************************
float AChunk::FastDistance(const FVector& V1, const FVector& V2)
{
	const float X = V1.X - V2.X;
	const float Y = V1.Y - V2.Y;
	const float Z = V1.Z - V2.Z;

	return FMath::Sqrt((X * X) + (Y * Y) + (Z * Z));
}

bool AChunk::GetRandomPointOnTerrain(FTransform& OutTransform) const
{
	const float X = FMath::RandRange(0, VERTEX_SIZE);
	const float Y = FMath::RandRange(0, VERTEX_SIZE);
	const float Z = HeightData[(X * (VERTEX_SIZE + 1)) + Y];

	const FVector ThisChunkLocation = GetActorLocation();
	const FVector2D ContinentalnessTestPoint = FVector2D(X + ThisChunkLocation.X, Y + ThisChunkLocation.Y);
	const float Continentalness = GetContinentalnessAtLocation(ContinentalnessTestPoint, true);
	if (Continentalness < 0.01f)
	{
		return false;
	}

	float ChunkOffset = (VERTEX_SIZE * VERTEX_DISTANCE_SCALE) * 0.5f;
	OutTransform.SetLocation(FVector((X * VERTEX_DISTANCE_SCALE) - ChunkOffset, (Y * VERTEX_DISTANCE_SCALE) - ChunkOffset, Z) + GetActorLocation());
	return true;
}

bool AChunk::IsWithinThreshold(float TestValue, float MinThreshold, float MaxThreshold)
{
	if (TestValue < MinThreshold || TestValue > MaxThreshold)
	{
		return false;
	}

	return true;
}

bool AChunk::IsWithinThreshold(const TArray<float>& TestValues, float MinThreshold, float MaxThreshold)
{
	for (float TestValue : TestValues)
	{
		if (!IsWithinThreshold(TestValue, MinThreshold, MaxThreshold))
		{
			return false;
		}
	}

	return true;
}

void AChunk::UpdateVisibility()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	APlayerController* LocalPlayerController = World->GetFirstPlayerController();
	if (LocalPlayerController == nullptr)
	{
		return;
	}

	APawn* LocalPawn = LocalPlayerController->GetPawn();
	if (LocalPawn == nullptr)
	{
		return;
	}

	UChunkInvokerComponent* ChunkInvoker = LocalPawn->FindComponentByClass<UChunkInvokerComponent>();
	if (ChunkInvoker == nullptr)
	{
		return;
	}

	const FVector FlattenedInvokerLocation(ChunkInvoker->GetComponentLocation().X, ChunkInvoker->GetComponentLocation().Y, 0.0f);
	const FVector FlattenedChunkLocation(this->GetActorLocation().X, this->GetActorLocation().Y, 0.0f);

	float Distance = FVector::Distance(FlattenedInvokerLocation, FlattenedChunkLocation);

	if (Distance >= ChunkInvoker->GetRenderDistanceCentimeters())
	{
		SetChunkHidden(true);
	}
	else
	{
		SetChunkHidden(false);
	}
}

//***************************************************************************************
//	Chunk Generation
//***************************************************************************************
void AChunk::GenerateTerrain(const TArray<FChunkData>& Neighbors)
{
	GenerateTerrainData(Neighbors);
	CreateTerrainMesh();
}

void AChunk::GenerateFeatures()
{
	//const FVector2D FlatWorldLocation(this->GetActorLocation().X, this->GetActorLocation().Y);
	const float StructureFactor = GetStructureFactorAtLocation(FVector2D(static_cast<float>(GetChunkLocation().X), static_cast<float>(GetChunkLocation().Y)), false);
	
	// TODO check if any of our neighbors contain a chunk, or if we are overlapping a structure
	const bool ChunkOverlapsStructure = false;
	if (StructureFactor > 0.0f || ChunkOverlapsStructure)
	{
		GenerateStructures();
		return;
	}

	GenerateDecorations();
}

void AChunk::GenerateStructures()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	FBiomeGenerationData* Biome = GetBiomeAtLocation(FVector2D(this->GetActorLocation().X, this->GetActorLocation().Y));
	if (Biome == nullptr)
	{
		return;
	}

	const int32 StructureIndex = FMath::RandRange(0, Biome->Structures.Num() - 1);
	if (!Biome->Structures.IsValidIndex(StructureIndex) || UKismetMathLibrary::RandomBoolWithWeight(Biome->Structures[StructureIndex].SpawnChance))
	{
		return;
	}
	
	const int32 ChunkHalf = FMath::RoundToInt32(17.0f * 0.5f); 
	const int32 TerrainDataIndex = (ChunkHalf * (VERTEX_SIZE + 1)) + ChunkHalf;
	const float ChunkHalfSize = CHUNK_SIZE_CENTIMETERS * 0.5f;
	const FVector SpawnLocation(this->GetActorLocation().X - ChunkHalfSize, this->GetActorLocation().Y - ChunkHalfSize, HeightData[TerrainDataIndex]);

	AChunkInvokerActor* ChunkInvoker = World->SpawnActor<AChunkInvokerActor>(AChunkInvokerActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
	if (ChunkInvoker == nullptr)
	{
		return;
	}
	
	ChunkInvoker->SetRenderDistance(4);
	ChunkInvoker->InvokeChunksNow();

	AActor* SpawnedStructure = World->SpawnActor<AActor>(Biome->Structures[StructureIndex].BlueprintClass, SpawnLocation, FRotator::ZeroRotator);
	if (SpawnedStructure == nullptr)
	{
		return;
	}
	
	AChunkManager* ChunkManager = AChunkManager::GetChunkManager();
	if (ChunkManager == nullptr)
	{
		return;
	}
	
	FVector StructureOrigin;
	FVector StructureBounds;
	SpawnedStructure->GetActorBounds(true, StructureOrigin, StructureBounds);
	
	const FIntVector2 StructureChunkBounds((StructureBounds.X / CHUNK_SIZE_CENTIMETERS) * 1.5f, (StructureBounds.Y / CHUNK_SIZE_CENTIMETERS) * 1.5f);

	// Clear Decorations is called on redecorate, which is called on flatten terrain
	//ChunkManager->ClearDecorationsAroundChunk(GetChunkLocation(), StructureChunkBounds);
	ChunkManager->FlattenTerrainAroundChunk(GetChunkLocation(), StructureChunkBounds * 2, SpawnLocation.Z);
	
	ChunkInvoker->Destroy();

	SpawnedStructure->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
}

void AChunk::GenerateDecorations()
{
	Async(EAsyncExecution::ThreadPool, [this]()
		{
			UWorld* World = GetWorld();
			if (World == nullptr)
			{
				return;
			}

			const float ChunkOffset = (VERTEX_SIZE * VERTEX_DISTANCE_SCALE) * 0.5f;
			const FVector ThisChunkLocation = GetActorLocation();

			// Use less than so nothing can spawn on the border between chunks, confusing the snow systems
			for (int32 X = 0; X < VERTEX_SIZE; ++X)
			{
				for (int32 Y = 0; Y < VERTEX_SIZE; ++Y)
				{
					const int32 TerrainDataIndex = (X * (VERTEX_SIZE + 1)) + Y;

					if (SurfaceData[TerrainDataIndex] == 1)
					{
						continue;
					}

					const FVector2D BiomeTestLocation = FVector2D((X * VERTEX_DISTANCE_SCALE) + ThisChunkLocation.X, (Y * VERTEX_DISTANCE_SCALE) + ThisChunkLocation.Y);
					FBiomeGenerationData* Biome = GetBiomeAtLocation(BiomeTestLocation);

					if (Biome == nullptr)
					{
						continue;
					}

					TArray<FSpawnQuery> SpawnQueryList
					{
						FSpawnQuery(Biome->Trees, 0.25f),
						FSpawnQuery(Biome->Nodes, 0.0f),
						FSpawnQuery(Biome->Collectables, -0.25f),
						FSpawnQuery(Biome->Lootables, -0.5f)
					};

					const float DecorationNoiseScale = 0.5f;
					const float Spawn = (FMath::FRand() * 2.0f) - 1.0f;//Noise.noise2D((X + (GridLocation.X * VERTEX_SIZE)) * DecorationNoiseScale, (Y + (GridLocation.Y * VERTEX_SIZE)) * DecorationNoiseScale);

					for (const FSpawnQuery& Query : SpawnQueryList)
					{
						if (Query.SpawnData.Spawnables.IsEmpty()
							|| !FMath::IsNearlyEqual(Spawn, Query.TestValue, Query.SpawnData.NoiseParameters.Tolerance))
						{
							continue;
						}

						const int32 SpawnDataIndex = FMath::RandRange(0, Query.SpawnData.Spawnables.Num() - 1);
						if (!UKismetMathLibrary::RandomBoolWithWeight(Query.SpawnData.Spawnables[SpawnDataIndex].SpawnChance))
						{
							continue;
						}

						const FVector SpawnLocation = FVector(
							(X * VERTEX_DISTANCE_SCALE) - ChunkOffset,
							(Y * VERTEX_DISTANCE_SCALE) - ChunkOffset,
							HeightData[TerrainDataIndex])
							+ ThisChunkLocation;
						const FRotator SpawnRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
						TSubclassOf<AActor> DecorationBlueprintClass = Query.SpawnData.Spawnables[SpawnDataIndex].BlueprintClass;
						AsyncTask(ENamedThreads::GameThread, [this, DecorationBlueprintClass, SpawnLocation, SpawnRotation]()
							{
								if (GetWorld())
								{
									AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(DecorationBlueprintClass, SpawnLocation, SpawnRotation);
									if (SpawnedActor)
									{
										SpawnedActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

									}
								}
							});

					}
				}
			}
		});
}

void AChunk::GenerateTerrainData(const TArray<FChunkData>& Neighbors)
{
	TArray<float> TopNeighborHeightData;
	TArray<float> BottomNeighborHeightData;
	TArray<float> LeftNeighborHeightData;
	TArray<float> RightNeighborHeightData;

	TArray<float> TopLeftNeighborHeightData;
	TArray<float> TopRightNeighborHeightData;
	TArray<float> BottomLeftNeighborHeightData;
	TArray<float> BottomRightNeighborHeightData;

	const FIntVector2 GridLocation = GetChunkLocation();

	// Iterate through neibors and store their edge height
	for (const FChunkData& ChunkData : Neighbors)
	{
		// Read Vertices data
		TArray<float> NeighborHeightData = ChunkData.HeightData;

		// Top
		if (ChunkData.GridLocation == GridLocation + FIntVector2(1, 0))
		{
			TopNeighborHeightData = NeighborHeightData;
		}
		// Bottom
		else if (ChunkData.GridLocation == GridLocation + FIntVector2(-1, 0))
		{
			BottomNeighborHeightData = NeighborHeightData;
		}
		// Left
		else if (ChunkData.GridLocation == GridLocation + FIntVector2(0, -1))
		{
			LeftNeighborHeightData = NeighborHeightData;
		}
		// Right
		else if (ChunkData.GridLocation == GridLocation + FIntVector2(0, 1))
		{
			RightNeighborHeightData = NeighborHeightData;
		}
		// Top Left
		else if (ChunkData.GridLocation == GridLocation + FIntVector2(1, -1))
		{
			TopLeftNeighborHeightData = NeighborHeightData;
		}
		// Top Right
		else if (ChunkData.GridLocation == GridLocation + FIntVector2(1, 1))
		{
			TopRightNeighborHeightData = NeighborHeightData;
		}
		// Bottom Left
		else if (ChunkData.GridLocation == GridLocation + FIntVector2(-1, -1))
		{
			BottomLeftNeighborHeightData = NeighborHeightData;
		}
		// Bottom Right
		else if (ChunkData.GridLocation == GridLocation + FIntVector2(-1, 1))
		{
			BottomRightNeighborHeightData = NeighborHeightData;
		}
	}

	// Use Neighbor edge height when generating this to ensure that there will be no gaps

	const FVector Location = GetActorLocation();
	for (uint32 X = 0; X <= VERTEX_SIZE; ++X)
	{
		for (uint32 Y = 0; Y <= VERTEX_SIZE; ++Y)
		{
			//const float NewZ = Noise.octave2D(static_cast<float>(X + Location.X) * NoiseScale, static_cast<float>(Y + Location.Y) * NoiseScale, 3) * ZScale;
			//const float Z = FMath::PerlinNoise2D(FVector2D((static_cast<float>(X) + Location.X) * NoiseScale, (static_cast<float>(Y) + Location.Y) * NoiseScale)) * ZScale;
			float Height;
			uint8 Surface;
			GetTerrainDataAtLocation(FVector2D((X * VERTEX_DISTANCE_SCALE) + Location.X, (Y * VERTEX_DISTANCE_SCALE) + Location.Y), Height, Surface);

			const int32 RowColumnSize = VERTEX_SIZE + 1;

			// Top
			if (!TopNeighborHeightData.IsEmpty() && X == VERTEX_SIZE)
			{
				Height = TopNeighborHeightData[Y];
			}
			// Bottom
			else if (!BottomNeighborHeightData.IsEmpty() && X == 0)
			{
				Height = BottomNeighborHeightData[Y + (RowColumnSize * VERTEX_SIZE)];
			}
			// Left
			else if (!LeftNeighborHeightData.IsEmpty() && Y == 0)
			{
				Height = LeftNeighborHeightData[(X * RowColumnSize) + VERTEX_SIZE];
			}
			// Right
			else if (!RightNeighborHeightData.IsEmpty() && Y == VERTEX_SIZE)
			{

				Height = RightNeighborHeightData[X * RowColumnSize];
			}
			// Top Left
			else if (!TopLeftNeighborHeightData.IsEmpty() && X == VERTEX_SIZE && Y == 0)
			{
				Height = TopLeftNeighborHeightData[VERTEX_SIZE];
			}
			// Top Right
			else if (!TopRightNeighborHeightData.IsEmpty() && X == VERTEX_SIZE && Y == VERTEX_SIZE)
			{
				Height = TopRightNeighborHeightData[0];
			}
			// Bottom Left
			else if (!BottomLeftNeighborHeightData.IsEmpty() && X == 0 && Y == 0)
			{
				Height = BottomLeftNeighborHeightData[(RowColumnSize * RowColumnSize) - 1];
			}
			// Bottom Right
			else if (!BottomRightNeighborHeightData.IsEmpty() && X == 0 && Y == VERTEX_SIZE)
			{
				Height = BottomRightNeighborHeightData[RowColumnSize * VERTEX_SIZE];
			}

			HeightData.Add(Height);
			SurfaceData.Add(Surface);
		}
	}
}

void AChunk::GenerateTerrainRuntimeData(TArray<FVector>& OutVertices, TArray<int32>& OutTriangles, TArray<FColor>& OutColors, TArray<FVector2D>& OutUV)
{
	OutVertices.Reset();
	OutTriangles.Reset();
	OutColors.Reset();
	OutUV.Reset();

	const float VertexOffset = (VERTEX_SIZE * VERTEX_DISTANCE_SCALE) * 0.5f;

	for (int32 X = 0; X <= VERTEX_SIZE; ++X)
	{
		for (int32 Y = 0; Y <= VERTEX_SIZE; ++Y)
		{
			const int32 ArrayIndex = (X * (VERTEX_SIZE + 1)) + Y;
			const FVector VertexLocation((X * VERTEX_DISTANCE_SCALE) - VertexOffset, (Y * VERTEX_DISTANCE_SCALE) - VertexOffset, HeightData[ArrayIndex]);

			OutVertices.Add(VertexLocation);

			switch (SurfaceData[ArrayIndex])
			{
			default:
				OutColors.Add(STONE_VERTEX_COLOR);
				break;
			case 1:
				OutColors.Add(STONE_VERTEX_COLOR);
				break;
			case 2:
				OutColors.Add(GRASS_VERTEX_COLOR);
				break;
			case 3:
				OutColors.Add(DIRT_VERTEX_COLOR);
				break;
			case 4:
				OutColors.Add(SAND_VERTEX_COLOR);
				break;
			case 5:
				OutColors.Add(GRAVEL_VERTEX_COLOR);
				break;
			case 6:
				OutColors.Add(SNOW_VERTEX_COLOR);
				break;
			case 7:
				OutColors.Add(RESERVED_VERTEX_COLOR);
				break;
			}
			OutUV.Add(FVector2D(X * UV_SCALE, Y * UV_SCALE));
		}
	}

	uint32 Vertex = 0;
	for (uint32 X = 0; X < VERTEX_SIZE; ++X)
	{
		for (uint32 Y = 0; Y < VERTEX_SIZE; ++Y)
		{
			OutTriangles.Add(Vertex);
			OutTriangles.Add(Vertex + 1);
			OutTriangles.Add(Vertex + VERTEX_SIZE + 1);

			OutTriangles.Add(Vertex + 1);
			OutTriangles.Add(Vertex + VERTEX_SIZE + 2);
			OutTriangles.Add(Vertex + VERTEX_SIZE + 1);

			++Vertex;
		}
		++Vertex;
	}
}

void AChunk::CreateTerrainMesh()
{
	MeshComponent->ClearMeshSection(0);
	
	TArray<FVector> Vertices;
	TArray<FColor> VertexColors;
	TArray<FVector2D> UV0;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FProcMeshTangent> Tangents;

	GenerateTerrainRuntimeData(Vertices, Triangles, VertexColors, UV0);

	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UV0, Normals, Tangents);

	MeshComponent->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, VertexColors, Tangents, true);
	MeshComponent->SetMaterial(0, ChunkMaterial);
}
