// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Chunk.generated.h"

UCLASS()
class WORLDGENERATION_API AChunk : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AChunk();
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	UFUNCTION()
	void Generate(const TArray<struct FChunkData>& Neighbors);

	UFUNCTION()
	void SetChunkHidden(bool Hidden);

	UFUNCTION()
	void OnLoadedTerrainData();

	void Save(struct FChunkData& OutChunkData, bool AlsoDestroy = false);
	void Load(const struct FChunkData& InChunkData);

	//***************************************************************************************
	//	Noise
	//***************************************************************************************
	static float GetContinentalnessAtLocation(const FVector2D& Location, bool UseRawValue = false);
	static float GetErosionAtLocation(const FVector2D& Location, bool UseRawValue = false);
	static float GetPeaksAndValleysAtLocation(const FVector2D& Location, bool UseRawValue = false);
	static float GetStructureFactorAtLocation(const FVector2D& Location, bool UseRawValue = false);

	//***************************************************************************************
	//	Chunk Setters
	//***************************************************************************************
	static void SetGenerationSeed(const uint32& InSeed);

	void SetChunkLocation(const FIntVector2& InLocation);
	void SetHeightData(const TArray<float>& InHeightData);
	void SetSurfaceData(const TArray<uint8>& InSurfaceData);

	void ClearAllAttachedDecorations();
	void Redecorate();
	
	//***************************************************************************************
	//	Chunk Getters
	//***************************************************************************************
	static uint32 GetGenerationSeed();

	static FName GetBiomeNameAtLocation(const FVector2D& Location);
	static struct FBiomeGenerationData* GetBiomeAtLocation(const FVector2D& Location);
	
	static void GetTerrainDataAtLocation(const FVector2D& Location, float& OutHeight, uint8& OutSurface);
	static float GetTerrainHeightAtLocation(const FVector2D& Location);
	
	FIntVector2 GetChunkLocation() const;
	bool IsChunkHidden() const;
	TArray<float> GetHeightData() const;
	TArray<uint8> GetSurfaceData() const;

	static int32 GetVertexSize();
	static float GetVertexDistanceScale();
	static float GetMaxHeight();

	//***************************************************************************************
	//	Chunk Helpers
	//***************************************************************************************
	static float FastDistance(const FVector& V1, const FVector& V2);

	bool GetRandomPointOnTerrain(FTransform& OutTransform) const;

	// Return if the TestValue is within the range of the Threshold
	static bool IsWithinThreshold(float TestValue, float MinThreshold, float MaxThreshold);
	// Returns false if only one element is outside of the given range
	static bool IsWithinThreshold(const TArray<float>& TestValues, float MinThreshold, float MaxThreshold);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(VisibleAnywhere)
	class UProceduralMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* WaterMeshComponent;

	UPROPERTY(VisibleAnywhere)
	class UChunkSaveComponent* SaveComponent;

	UPROPERTY(Replicated, ReplicatedUsing = OnLoadedTerrainData)
	TArray<float> HeightData;

	UPROPERTY(Replicated, ReplicatedUsing = OnLoadedTerrainData)
	TArray<uint8> SurfaceData;

	bool ChunkHidden;


	//TODO remove this
	UPROPERTY(VisibleAnywhere)
	FIntVector2 ChunkLoc;

	//***************************************************************************************
	//	Chunk Visibility
	//***************************************************************************************
	FTimerHandle UpdateVisibilityTimerHandle;
	
	UFUNCTION()
	void UpdateVisibility();

	//***************************************************************************************
	//	Chunk Generation
	//***************************************************************************************

	// Generates the terrain (Height, Surface Type)
	void GenerateTerrain(const TArray<FChunkData>& Neighbors);

	// Handles both feature types (Structures, and Decorations)
	void GenerateFeatures();
	void GenerateStructures();
	// Generates the decorations (Trees, Rocks, Plants)
	void GenerateDecorations();

	void GenerateTerrainData(const TArray<FChunkData>& Neighbors);
	void GenerateTerrainRuntimeData(TArray<FVector>& OutVertices, TArray<int32>& OutTriangles, TArray<FColor>& OutColors, TArray<FVector2D>& OutUV);

	void CreateTerrainMesh();


};
