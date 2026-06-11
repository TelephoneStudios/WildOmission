// Copyright Telephone Studios. All Rights Reserved.


#include "Deployables/Deployable.h"
#include "Components/BuildAnchorComponent.h"
#include "Actors/DeployablePreview.h"
#include "ChunkManager.h"
#include "Components/ChunkInvokerComponent.h"
#include "Actors/Chunk.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NavModifierComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DamageEvents.h"
#include "UObject/ConstructorHelpers.h"

static UNiagaraSystem* DustSystem = nullptr;

// Sets default values
ADeployable::ADeployable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = false;
	SetNetUpdateFrequency(5.0f);
	NetDormancy = DORM_DormantAll;

	DeployableRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DeployableRootComponent"));
	DeployableRootComponent->SetMobility(EComponentMobility::Type::Stationary);
	RootComponent = DeployableRootComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetMobility(EComponentMobility::Type::Stationary);
	MeshComponent->SetupAttachment(DeployableRootComponent);
	MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel9, ECollisionResponse::ECR_Overlap);

	NavigationModifier = CreateDefaultSubobject<UNavModifierComponent>(TEXT("NavigationModifier"));

	static ConstructorHelpers::FClassFinder<UNavArea> DefaultNavArea(TEXT("/Script/CoreUObject.Class'/Script/NavigationSystem.NavArea_Default'"));
	if (DefaultNavArea.Succeeded())
	{
		NavigationModifier->SetAreaClass(DefaultNavArea.Class);
	}

	MaxDurability = 100.0f;

	Identifier = NAME_None;
	
	ItemID = NAME_None;

	bCanSpawnOnGround = true;
	bCanSpawnOnFloor = false;
	bCanSpawnOnCeiling = false;
	bCanSpawnOnWall = false;
	CanSpawnOnAnchor = EBuildAnchorType::None;
	bFollowsSurfaceNormal = false;
	bCanRotate = false;

	static ConstructorHelpers::FObjectFinder<USoundBase> DefaultPlacementSound(TEXT("/Game/Deployables/Audio/Deployable_Placement_Cue"));
	if (DefaultPlacementSound.Succeeded())
	{
		PlacementSound = DefaultPlacementSound.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<USoundBase> DefaultDestructionSound(TEXT("/Game/Deployables/Audio/Deployable_Destruction_Cue"));
	if (DefaultDestructionSound.Succeeded())
	{
		DestructionSound = DefaultDestructionSound .Object;
	}

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DustBlueprint(TEXT("/Game/WildOmissionCore/Art/Effects/NS_BoundsDust"));
	if (DustBlueprint.Succeeded())
	{
		DustSystem = DustBlueprint.Object;
	}
}

// Called when the game starts or when spawned
void ADeployable::BeginPlay()
{
	Super::BeginPlay();
	
	//AChunkManager::HandleActorRenderDistanceVisibility(GetWorld(), this);
}

void ADeployable::OnLoadComplete_Implementation()
{
	CurrentDurability = MaxDurability * NormalizedDurability;
}

void ADeployable::OnSpawn()
{
	int32 ChunkLocationX = this->GetActorLocation().X / (AChunk::GetVertexSize() * AChunk::GetVertexDistanceScale());
	int32 ChunkLocationY = this->GetActorLocation().Y / (AChunk::GetVertexSize() * AChunk::GetVertexDistanceScale());

	AChunkManager* ChunkManager = AChunkManager::GetChunkManager();
	if (ChunkManager == nullptr)
	{
		return;
	}

	FSpawnedChunk SpawnedChunk;
	if (!ChunkManager->GetSpawnedChunk(FIntVector2(ChunkLocationX, ChunkLocationY), SpawnedChunk) || !IsValid(SpawnedChunk.Chunk))
	{
		return;
	}

	this->AttachToActor(SpawnedChunk.Chunk, FAttachmentTransformRules::KeepWorldTransform);

	// Broadcast Overlap
	TArray<UPrimitiveComponent*> OverlappingComponents;
	GetOverlappingComponents(OverlappingComponents);
	for (UPrimitiveComponent* OverlappingComponent : OverlappingComponents)
	{
		if (OverlappingComponent == nullptr || !OverlappingComponent->OnComponentBeginOverlap.IsBound() || Cast<ADeployablePreview>(OverlappingComponent->GetOwner()))
		{
			continue;
		}

		OverlappingComponent->OnComponentBeginOverlap.Broadcast(OverlappingComponent, this, MeshComponent, INDEX_NONE, false, FHitResult());
	}

	// Set Durability
	CurrentDurability = MaxDurability;

	// Placement Effects
	Multi_PlayPlacementEffects();
}

bool ADeployable::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);

	return AChunkManager::IsActorNetRelevent(this, ViewTarget);
}

void ADeployable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADeployable, CurrentDurability);
}

void ADeployable::Serialize(FArchive& Ar)
{
	if (Ar.IsSaving())
	{
		NormalizedDurability = CurrentDurability / MaxDurability;
	}

	Super::Serialize(Ar);
}

void ADeployable::Destroyed()
{
	Super::Destroyed();
}

void ADeployable::ApplyWindDamage(AActor* WindCauser, float DamageMultiplier)
{
	float DamageToApply = BaseWindDamage * DamageMultiplier * GetWorld()->GetDeltaSeconds();
	FRadialDamageEvent DamageEvent;
	this->TakeDamage(DamageToApply, DamageEvent, nullptr, WindCauser);
}

float ADeployable::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	CurrentDurability = FMath::Clamp(CurrentDurability - DamageAmount, 0.0f, MaxDurability);
	
	if (CurrentDurability < KINDA_SMALL_NUMBER)
	{
		OnDeployableDestroyed();
		this->Destroy();
	}

	FlushNetDormancy();
	return DamageAmount;
}

void ADeployable::OnDeployableDestroyed()
{
	Multi_PlayDestructionEffects();
}

void ADeployable::Rotate()
{
	const FRotator CurrentRotation = this->GetActorRotation();
	this->SetActorRotation(CurrentRotation + FRotator(0.0f, 90.0f, 0.0f));
}

UStaticMesh* ADeployable::GetMesh() const
{
	return MeshComponent->GetStaticMesh();
}

FTransform ADeployable::GetMeshTransform() const
{
	return MeshComponent->GetRelativeTransform();
}

AChunk* ADeployable::GetChunk() const
{
	AChunk* OwnerChunk = Cast<AChunk>(this->GetAttachParentActor());
	return OwnerChunk;
}

FIntVector2 ADeployable::GetChunkLocation() const
{
	AChunk* OwnerChunk = GetChunk();
	if (!IsValid(OwnerChunk))
	{
		return FIntVector2();
	}

	return OwnerChunk->GetChunkLocation();
}

float ADeployable::GetCurrentDurability()
{
	return CurrentDurability;
}

float ADeployable::GetMaxDurability()
{
	return MaxDurability;
}

FName ADeployable::GetIdentifier() const
{
	return Identifier;
}

FName ADeployable::GetItemID() const
{
	return ItemID;
}

bool ADeployable::CanSpawnOnGround() const
{
	return bCanSpawnOnGround;
}

bool ADeployable::CanSpawnOnFloor() const
{
	return bCanSpawnOnFloor;
}

bool ADeployable::CanSpawnOnCeiling() const
{
	return bCanSpawnOnCeiling;
}

bool ADeployable::CanSpawnOnWall() const
{
	return bCanSpawnOnWall;
}

TEnumAsByte<EBuildAnchorType> ADeployable::CanSpawnOnBuildAnchor() const
{
	return CanSpawnOnAnchor;
}

bool ADeployable::FollowsSurfaceNormal() const
{
	return bFollowsSurfaceNormal;
}

bool ADeployable::CanRotate() const
{
	return bCanRotate;
}

void ADeployable::Multi_PlayPlacementEffects_Implementation()
{
	SpawnDustEffects();

	UGameplayStatics::PlaySoundAtLocation(GetWorld(), PlacementSound, GetActorLocation());
}

void ADeployable::Multi_PlayDestructionEffects_Implementation()
{
	SpawnDustEffects();
	
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), DestructionSound, GetActorLocation());
}

void ADeployable::SpawnDustEffects()
{
	if (DustSystem == nullptr || MeshComponent == nullptr)
	{
		return;
	}

	UStaticMesh* MeshComponentMesh = MeshComponent->GetStaticMesh();
	if (MeshComponentMesh == nullptr)
	{
		return;
	}

	FVector Origin = MeshComponentMesh->GetBounds().Origin + GetActorLocation();
	FVector BoxExtent = MeshComponentMesh->GetBounds().BoxExtent;
	int32 BoxSurfaceArea = FMath::RoundToInt32(BoxExtent.X + BoxExtent.Y + BoxExtent.Z);

	UNiagaraComponent* SpawnedDust = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), DustSystem, Origin, GetActorRotation(), FVector(1.0f), true, false);
	if (SpawnedDust == nullptr)
	{
		return;
	}

	SpawnedDust->SetVectorParameter(TEXT("BoundingBox"), BoxExtent);
	SpawnedDust->SetIntParameter(TEXT("BoundingBoxArea"), BoxSurfaceArea);
	SpawnedDust->Activate(true);
}