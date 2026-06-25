// Copyright Telephone Studios. All Rights Reserved.


#include "WorldItem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PlayerInventoryComponent.h"
#include "Components/InventoryManipulatorComponent.h"
#include "Components/TimerDespawnComponent.h"
#include "Actors/Chunk.h"
#include "ChunkManager.h"
#include "Structs/ItemStat.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AWorldItem::AWorldItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Set this actor to replicate
	bReplicates = true;
	SetReplicateMovement(true);

	// Setup default values
	Item.Name = TEXT("Item");
	Item.Quantity = 1;

	LastClump = nullptr;

	ItemRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ItemRootComponent"));
	RootComponent = ItemRootComponent;

	// Create static mesh component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetStaticMesh(nullptr);
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel11, ECollisionResponse::ECR_Block);
	MeshComponent->SetNotifyRigidBodyCollision(true);
	MeshComponent->SetIsReplicated(true);

	// Setup Despawner
	DespawnComponent = CreateDefaultSubobject<UTimerDespawnComponent>(TEXT("DespawnComponent"));
	DespawnComponent->SetDespawnTime(300.0f);

	static ConstructorHelpers::FObjectFinder<USoundBase> ClumpSoundObject(TEXT("/Game/Inventory/Audio/ItemClump_Cue"));
	if (ClumpSoundObject.Succeeded())
	{
		ClumpSound = ClumpSoundObject.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> PickupSoundObject(TEXT("/Game/WildOmissionCore/Characters/Human/Audio/Pickup/Pickup_Cue"));
	if (PickupSoundObject.Succeeded())
	{
		PickupSound = PickupSoundObject.Object;
	}
}

// Called when the game starts or when spawned
void AWorldItem::BeginPlay()
{
	Super::BeginPlay();
	
	if (!HasAuthority())
	{
		return;
	}

	MeshComponent->SetMassOverrideInKg(NAME_None, 20.0f);
	MeshComponent->OnComponentHit.AddDynamic(this, &AWorldItem::OnComponentHit);

	UpdateAttachedChunk();
}

void AWorldItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWorldItem, Item);
}

void AWorldItem::Interact(AActor* Interactor)
{
	UInventoryComponent* InteractorInventoryComponent = Interactor->FindComponentByClass<UInventoryComponent>();
	if (InteractorInventoryComponent == nullptr)
	{
		return;
	}

	// Add to their inventory
	InteractorInventoryComponent->AddItem(Item);

	// Play Pickup sound
	Multi_PlayPickupSound();

	// Destroy this Item
	Destroy();
}

FString AWorldItem::PromptText()
{
	FString ItemDisplayName;

	FItemData* ItemData = UInventoryComponent::GetItemData(Item.Name);
	if (ItemData == nullptr)
	{
		return FString::Printf(TEXT("pickup %s"), *Item.Name.ToString());
	}

	ItemDisplayName = ItemData->DisplayName;

	return FString::Printf(TEXT("pickup %s"), *ItemDisplayName);
}

FName AWorldItem::GetIdentifier() const
{
	return TEXT("WorldItem");
}

void AWorldItem::SetItem(const FInventoryItem& InItem)
{
	Item = InItem;
	OnRep_Item();
}

void AWorldItem::AddImpulse(FVector Impulse)
{
	MeshComponent->AddImpulse(Impulse);
}

void AWorldItem::SetLastClump(AWorldItem* InLastClump)
{
	LastClump = InLastClump;
}

AWorldItem* AWorldItem::GetLastClump() const
{
	return LastClump;
}

FInventoryItem AWorldItem::GetItem() const
{
	return Item;
}

void AWorldItem::UpdateAttachedChunk()
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

void AWorldItem::OnRep_Item()
{
	FItemData* NewItemData = UInventoryComponent::GetItemData(Item.Name);
	if (NewItemData == nullptr)
	{
		return;
	}

	MeshComponent->SetStaticMesh(NewItemData->Mesh);
}

void AWorldItem::OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	AWorldItem* OtherWorldItem = Cast<AWorldItem>(OtherActor);
	if (OtherWorldItem == nullptr || OtherWorldItem == LastClump)
	{
		return;
	}

	FInventoryItem OurItem = this->GetItem();
	FInventoryItem OtherItem = OtherWorldItem->GetItem();
	FItemData* ItemData = UInventoryComponent::GetItemData(OurItem.Name);
	if (OtherItem.Name != OurItem.Name || ItemData == nullptr || OurItem.Quantity >= ItemData->StackSize || ItemData->StackSize == 1)
	{
		return;
	}

	OurItem.Quantity = OtherItem.Quantity + OurItem.Quantity;
	int32 RemainingQuantity = 0;

	if (OurItem.Quantity > ItemData->StackSize)
	{
		RemainingQuantity = OurItem.Quantity - ItemData->StackSize;
		OurItem.Quantity = ItemData->StackSize;
		
		OtherItem.Quantity = RemainingQuantity;
		OtherWorldItem->SetItem(OtherItem);
		OtherWorldItem->SetLastClump(this);

		this->SetItem(OurItem);
	}
	else
	{
		OtherWorldItem->Destroy();

		this->SetItem(OurItem);
	}

	Multi_PlayClumpSound();
}

void AWorldItem::OnLoadComplete_Implementation()
{
	OnRep_Item();
}

void AWorldItem::Multi_PlayClumpSound_Implementation()
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), ClumpSound, GetActorLocation());
}

void AWorldItem::Multi_PlayPickupSound_Implementation()
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), PickupSound, GetActorLocation());
}