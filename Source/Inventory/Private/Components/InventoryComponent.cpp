// Copyright Telephone Studios. All Rights Reserved.


#include "Components/InventoryComponent.h"
#include "Components/InventoryManipulatorComponent.h"
#include "Blueprint/UserWidget.h"
#include "WorldItem.h"
#include "Net/UnrealNetwork.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "Log.h"

static UDataTable* ItemDataTable = nullptr;

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	SetIsReplicatedByDefault(true);

	DisplayName = TEXT("Inventory");

	SlotCount = 24;
	LoadedFromSave = false;

	// We have to add this stupid ugly check here, because unreal engine is a buggy broken mess somethimes.
	if (GetWorld())
	{
		static ConstructorHelpers::FObjectFinder<UDataTable> ItemDataTableBlueprint(TEXT("/Game/Inventory/DataTables/DT_Items"));
		if (ItemDataTableBlueprint.Succeeded())
		{
			ItemDataTable = ItemDataTableBlueprint.Object;
		}
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> DefaultWidgetBlueprint(TEXT("/Game/Deployables/UI/WBP_ContainerBase"));
	if (DefaultWidgetBlueprint.Succeeded())
	{
		WidgetClass = DefaultWidgetBlueprint.Class;
	}
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UInventoryComponent, ServerState, COND_OwnerOnly);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!GetOwner()->HasAuthority() || ServerState.Slots.Num() == SlotCount)
	{
		return;
	}

	// Initialize slots
	ServerState.Slots.SetNum(SlotCount, false);
	for (int32 i = 0; i < SlotCount; ++i)
	{
		ServerState.Slots[i].Index = i;
	}

	LoadedFromSave = false;

	OnRep_ServerState();
}

void UInventoryComponent::AddItem(const FInventoryItem& ItemToAdd, bool FromHarvesting, AActor* ActorToSpawnDropedItems, bool ForceClientUpdate)
{
	AActor* OwningActor = GetOwner();
	if (OwningActor == nullptr)
	{
		return;
	}

	const bool UseServerState = OwningActor->HasAuthority();

	// Create the Item Update
	if (UseServerState)
	{
		FInventoryItemUpdate AdditionItemUpdate;
		AdditionItemUpdate.Time = GetWorld()->GetRealTimeSeconds();
		AdditionItemUpdate.Addition = true;
		AdditionItemUpdate.Item = ItemToAdd;
		ServerState.Updates.Add(AdditionItemUpdate);
	}
	
	// The amount of items we were unable to add
	int32 Remaining;
	
	// The amount of items we successfully added
	int32 AmountAdded;
	
	bool AddSuccess = AddItemToSlots(ItemToAdd, Remaining);

	// Calculate how many were added and add that amount to our contents
	AmountAdded = ItemToAdd.Quantity - Remaining;
	UseServerState ? ServerState.Contents.AddItem(ItemToAdd.Name, AmountAdded) : Contents.AddItem(ItemToAdd.Name, AmountAdded);

	if (FromHarvesting && OnItemHarvested.IsBound())
	{
		FInventoryItem ItemHarvested;
		ItemHarvested.Name = ItemToAdd.Name;
		ItemHarvested.Quantity = AmountAdded;
		OnItemHarvested.Broadcast(ItemHarvested);
	}

	if (UseServerState == true && AddSuccess == false)
	{
		FInventoryItem DroppedItem = ItemToAdd;
		DroppedItem.Quantity = Remaining;

		FInventoryItemUpdate DroppedItemUpdate;
		DroppedItemUpdate.Time = GetWorld()->GetRealTimeSeconds();
		DroppedItemUpdate.Addition = false;
		DroppedItemUpdate.Item = DroppedItem;
		ServerState.Updates.Add(DroppedItemUpdate);

		AActor* DropperActor = nullptr;
		if (ActorToSpawnDropedItems == nullptr)
		{
			DropperActor = GetOwner();
		}
		else
		{
			DropperActor = ActorToSpawnDropedItems;
		}
		SpawnWorldItem(GetWorld(), DroppedItem, DropperActor);
	}
	
	if (ForceClientUpdate)
	{
		GetOwner()->ForceNetUpdate();
	}

	OnRep_ServerState();
}

void UInventoryComponent::RemoveItem(const FInventoryItem& ItemToRemove)
{
	int32 Remaining;
	int32 AmountSuccessfullyRemoved;
	
	RemoveItemFromSlots(ItemToRemove.Name, ItemToRemove.Quantity, Remaining);
	AmountSuccessfullyRemoved = ItemToRemove.Quantity - Remaining;

	// Remove from contents
	ServerState.Contents.RemoveItem(ItemToRemove.Name, AmountSuccessfullyRemoved);

	// Call inventory change
	OnRep_ServerState();
}

void UInventoryComponent::SlotInteraction(const int32& SlotIndex, UInventoryManipulatorComponent* Manipulator, bool QuickMove, bool Primary)
{
	// Create an interaction
	FInventorySlotInteraction CurrentInteraction;
	CurrentInteraction.SlotIndex = SlotIndex;
	CurrentInteraction.Manipulator = Manipulator;
	CurrentInteraction.QuickMove = QuickMove;
	CurrentInteraction.Primary = Primary;
	CurrentInteraction.Time = GetWorld()->GetRealTimeSeconds();

	// Send it to the server
	Server_SlotInteraction(CurrentInteraction);

	// Add it to our unacknowlaged interactions
	UnacknowalgedInteractions.Add(CurrentInteraction);

	if (GetOwner()->HasAuthority())
	{
		return;
	}

	CurrentInteraction.QuickMove ? HandleItemQuickMove(CurrentInteraction) : HandleItemSelection(CurrentInteraction);

	BroadcastInventoryUpdate();
}

FItemData* UInventoryComponent::GetItemData(const FName& ItemID)
{
	if (ItemDataTable == nullptr)
	{
		return nullptr;
	}

	static const FString ContextString(TEXT("Item Data Context"));

	return ItemDataTable->FindRow<FItemData>(ItemID, ContextString, true);
}

void UInventoryComponent::SpawnWorldItem(UWorld* WorldContextObject, const FInventoryItem& ItemToSpawn, AActor* SpawningActor)
{
	if (WorldContextObject == nullptr || SpawningActor == nullptr)
	{
		return;
	}

	FVector SpawningActorOrigin;
	FVector SpawningActorBounds;
	SpawningActor->GetActorBounds(true, SpawningActorOrigin, SpawningActorBounds);

	// Spawn a world item actor
	const FVector SpawnLocation = SpawningActor->GetActorLocation() + FVector(0.0f, 0.0f, SpawningActorBounds.Z * 0.5f);
	const FRotator SpawnRotation = FRotator::ZeroRotator;
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = SpawningActor;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AWorldItem* WorldItem = WorldContextObject->SpawnActor<AWorldItem>(SpawnLocation, SpawnRotation, SpawnParams);
	if (WorldItem == nullptr)
	{
		return;
	}

	// Update world items properties
	WorldItem->SetItem(ItemToSpawn);

	const FVector PhysicsImpulse = SpawningActor->GetActorForwardVector() * 5000.0f;
	
	WorldItem->AddImpulse(PhysicsImpulse);
}

TArray<FName> UInventoryComponent::GetAllItems()
{
	if (ItemDataTable == nullptr)
	{
		UE_LOG(LogInventory, Display, TEXT("Cannot get all item data, the item data table is missing."));
		TArray<FName> OutEmptyList;
		return OutEmptyList;
	}

	return ItemDataTable->GetRowNames();
}

void UInventoryComponent::OnLoadComplete_Implementation()
{
	if (ServerState.Slots.Num() < SlotCount)
	{
		UE_LOG(LogInventory, Warning, TEXT("Container Slot Count Invalid Resizing, SlotsNum: %i, SlotCount: %i"), ServerState.Slots.Num(), SlotCount);
		for (int32 i = ServerState.Slots.Num(); i < SlotCount; ++i)
		{
			FInventorySlot Slot;
			Slot.Index = i;
			ServerState.Slots.Add(Slot);
		}
	}
	else if (ServerState.Slots.Num() > SlotCount)
	{
		UE_LOG(LogInventory, Warning, TEXT("Container Slot Count Invalid Resizing, SlotsNum: %i, SlotCount: %i"), ServerState.Slots.Num(), SlotCount);
		for (int32 i = SlotCount; i < ServerState.Slots.Num(); i++)
		{
			if (ServerState.Slots[i].IsEmpty())
			{
				ServerState.Slots.RemoveAt(ServerState.Slots.Num() - 1);
				continue;
			}
			FInventoryItem SlotItem = ServerState.Slots[i].Item;
			ServerState.Contents.RemoveItem(SlotItem.Name, SlotItem.Quantity);
			UInventoryComponent::SpawnWorldItem(GetWorld(), SlotItem, nullptr);
			ServerState.Slots.RemoveAt(ServerState.Slots.Num() - 1);
		}
		ServerState.Slots.Shrink();
	}

	ServerState.Contents = Contents;
	ServerState.Slots = Slots;

	LoadedFromSave = true;
}

void UInventoryComponent::HandleItemSelection(const FInventorySlotInteraction& Interaction)
{
	AActor* OwningActor = GetOwner();
	if (OwningActor == nullptr)
	{
		return;
	}
	
	bool UseServerState = OwningActor->HasAuthority();

	if (!Interaction.Manipulator->IsDragging())
	{
		if (Interaction.Primary)
		{
			DragAll(Interaction.SlotIndex, Interaction.Manipulator, UseServerState ? ServerState.Slots : Slots, UseServerState ? ServerState.Contents : Contents);
		}
		else
		{
			DragSplit(Interaction.SlotIndex, Interaction.Manipulator, UseServerState ? ServerState.Slots : Slots, UseServerState ? ServerState.Contents : Contents);
		}
	}
	else
	{
		if (Interaction.Primary)
		{
			DropAll(Interaction.SlotIndex, Interaction.Manipulator, UseServerState ? ServerState.Slots : Slots, UseServerState ? ServerState.Contents : Contents);
		}
		else
		{
			DropSingle(Interaction.SlotIndex, Interaction.Manipulator, UseServerState ? ServerState.Slots : Slots, UseServerState ? ServerState.Contents : Contents);
		}
	}
}

void UInventoryComponent::HandleItemQuickMove(const FInventorySlotInteraction& Interaction)
{
	AActor* OwningActor = GetOwner();
	if (OwningActor == nullptr)
	{
		return;
	}

	bool UseServerState = OwningActor->HasAuthority();
	
	bool WithinPlayerInventory = Interaction.Manipulator->GetOwnersInventory() == this;
	bool ContainerOpen = Interaction.Manipulator->GetOpenContainer() != nullptr;
	
	FInventorySlot& InteractionSlot = UseServerState ? ServerState.Slots[Interaction.SlotIndex] : Slots[Interaction.SlotIndex];
	if (InteractionSlot.IsEmpty())
	{
		return;
	}

	if (WithinPlayerInventory) // Interaction from within the players inventory
	{
		if (ContainerOpen) // Move from the player's inventory to the open container
		{
			// Store the slot's item information
			FInventoryItem MovingItem = UseServerState ? ServerState.Slots[Interaction.SlotIndex].Item : Slots[Interaction.SlotIndex].Item;

			// Remove the item from the player's inventory
			UseServerState ? ServerState.Contents.RemoveItem(MovingItem.Name, MovingItem.Quantity) : Contents.RemoveItem(MovingItem.Name, MovingItem.Quantity);
			UseServerState ? ServerState.Slots[Interaction.SlotIndex].ClearItem() : Slots[Interaction.SlotIndex].ClearItem();

			// Add item to the container's inventory
			Interaction.Manipulator->GetOpenContainer()->AddItem(MovingItem, false, GetOwner());
		}
		else // Move within the player's inventory
		{
			// Store the slot's item information
			FInventoryItem MovingItem = UseServerState ? ServerState.Slots[Interaction.SlotIndex].Item : Slots[Interaction.SlotIndex].Item;
			
			// Clear this slot before performing quick move
			UseServerState ? ServerState.Slots[Interaction.SlotIndex].ClearItem() : Slots[Interaction.SlotIndex].ClearItem();

			// Find the best availible slot and add the item to it
			const bool FromToolbar = Interaction.SlotIndex < 6;
			int32 Remaining = 0;
			const bool AddedAll = AddItemToSlots(MovingItem, Remaining, FromToolbar);

			// This was added so items don't disapear if the inventory is full 6/14/2024 at like 6:43 AM
			if (AddedAll == false && FromToolbar == true)
			{
				MovingItem.Quantity = Remaining;
				(void*)AddItemToSlots(MovingItem, Remaining);
			}
		}
	}
	else if (ContainerOpen && !WithinPlayerInventory) // Interaction from within an open container to the player's inventory
	{
		// Store the slot's item information
		FInventoryItem MovingItem = UseServerState ? ServerState.Slots[Interaction.SlotIndex].Item : Slots[Interaction.SlotIndex].Item;

		// Remove the item from this container
		UseServerState ? ServerState.Contents.RemoveItem(MovingItem.Name, MovingItem.Quantity) : Contents.RemoveItem(MovingItem.Name, MovingItem.Quantity);
		UseServerState ? ServerState.Slots[Interaction.SlotIndex].ClearItem() : Slots[Interaction.SlotIndex].ClearItem();

		// Add the item to the player's inventory
		Interaction.Manipulator->GetOwnersInventory()->AddItem(MovingItem);
	}
}

void UInventoryComponent::Server_SlotInteraction_Implementation(const FInventorySlotInteraction& Interaction)
{
	// Update our server state
	ServerState.LastInteraction = Interaction;

	Interaction.QuickMove ? HandleItemQuickMove(Interaction) : HandleItemSelection(Interaction);

	OnRep_ServerState();
}

FInventoryItem* UInventoryComponent::FindItemWithUniqueID(const uint32& UniqueID)
{
	AActor* OwningActor = GetOwner();
	if (OwningActor == nullptr)
	{
		return nullptr;
	}

	const bool UseServerState = OwningActor->HasAuthority();

	FInventoryItem* FoundItem = nullptr;
	TArray<FInventorySlot>& SlotArray = UseServerState ? ServerState.Slots : Slots;
	for (FInventorySlot& Slot : SlotArray)
	{
		if (Slot.IsEmpty())
		{
			continue;
		}
		if (Slot.Item.UniqueID != UniqueID)
		{
			continue;
		}
		
		FoundItem = &Slot.Item;
		break;
	}

	return FoundItem;
}

FInventorySlot* UInventoryComponent::FindSlotContainingItem(const FName& ItemToFind)
{
	AActor* OwningActor = GetOwner();
	if (OwningActor == nullptr)
	{
		return nullptr;
	}

	const bool UseServerState = OwningActor->HasAuthority();

	FInventorySlot* FoundSlot = nullptr;
	TArray<FInventorySlot>& SlotArray = UseServerState ? ServerState.Slots : Slots;
	for (FInventorySlot& Slot : SlotArray)
	{
		if (Slot.IsEmpty())
		{
			continue;
		}
		if (Slot.Item.Name != ItemToFind)
		{
			continue;
		}

		FoundSlot = &Slot;
		break;
	}

	return FoundSlot;
}

FString UInventoryComponent::GetDisplayName() const
{
	return DisplayName;
}

UClass* UInventoryComponent::GetWidgetClass() const
{
	return WidgetClass.Get();
}

FInventoryContents* UInventoryComponent::GetContents()
{
	return &Contents;
}

FInventorySlot* UInventoryComponent::GetSlot(const int32& SlotIndex)
{
	AActor* OwningActor = GetOwner();
	if (OwningActor == nullptr)
	{
		return nullptr;
	}
	
	const bool UseServerState = OwningActor->HasAuthority();
	
	if (UseServerState ? SlotIndex >= ServerState.Slots.Num() : SlotIndex > Slots.Num())
	{
		return nullptr;
	}
	else if (SlotIndex < 0)
	{
		return nullptr;
	}

	return UseServerState? &ServerState.Slots[SlotIndex] :  &Slots[SlotIndex];
}

uint8 UInventoryComponent::GetSlotCount() const
{
	return SlotCount;
}

TArray<FInventorySlot>& UInventoryComponent::GetSlots()
{
	return Slots;
}

void UInventoryComponent::RequestInventoryRefresh()
{
	OnRep_ServerState();
}

void UInventoryComponent::SetDisplayName(const FString& NewDisplayName)
{
	DisplayName = NewDisplayName;
}

void UInventoryComponent::SetSlotCount(const uint8& InSlotCount)
{
	SlotCount = InSlotCount;
}

bool UInventoryComponent::WasLoadedFromSave() const
{
	return LoadedFromSave;
}

TArray<uint8> UInventoryComponent::Save()
{
	TArray<uint8> Bytes;

	FMemoryWriter MemoryWriter(Bytes);
	FObjectAndNameAsStringProxyArchive InventoryArchive(MemoryWriter, true);
	InventoryArchive.ArIsSaveGame = true;
	this->Serialize(InventoryArchive);

	return Bytes;
}

void UInventoryComponent::Load(const TArray<uint8>& InSave)
{
	FMemoryReader MemoryReader(InSave);
	FObjectAndNameAsStringProxyArchive InventoryArchive(MemoryReader, true);
	InventoryArchive.ArIsSaveGame = true;
	this->Serialize(InventoryArchive);

	OnLoadComplete_Implementation();
	OnRep_ServerState();
}

void UInventoryComponent::OnRep_ServerState()
{
	ClearAcknowlagedInteractions(ServerState.LastInteraction);
	
	TArray<FInventoryItemUpdate> UnacknowlagedUpdates;
	GetUnacknowlagedUpdates(UnacknowlagedUpdates);
	BroadcastAllUpdateNotifications(UnacknowlagedUpdates);

	if (GetOwner()->HasAuthority() || UnacknowalgedInteractions.Num() == 0 || ServerState.LastInteraction.Time == 0)
	{
		Slots = ServerState.Slots;
		Contents = ServerState.Contents;
	}

	BroadcastInventoryUpdate();
}

void UInventoryComponent::ClearAcknowlagedInteractions(const FInventorySlotInteraction& LastInteraction)
{
	TArray<FInventorySlotInteraction> NewInteractionsList;

	for (const FInventorySlotInteraction& Interaction : UnacknowalgedInteractions)
	{
		if (Interaction.Time > LastInteraction.Time)
		{
			NewInteractionsList.Add(Interaction);
		}
	}

	UnacknowalgedInteractions = NewInteractionsList;
}

void UInventoryComponent::GetUnacknowlagedUpdates(TArray<FInventoryItemUpdate>& OutUpdatesList)
{
	for (const FInventoryItemUpdate& Update : ServerState.Updates)
	{
		if (Update.Time <= LastClientAcknowlagedItemUpdate.Time)
		{
			continue;
		}
		OutUpdatesList.Add(Update);
	}

	if (ServerState.Updates.IsEmpty())
	{
		return;
	}
	
	LastClientAcknowlagedItemUpdate = ServerState.Updates[ServerState.Updates.Num() - 1];
}

void UInventoryComponent::BroadcastAllUpdateNotifications(const TArray<FInventoryItemUpdate>& UpdatesList)
{
	for (const FInventoryItemUpdate& ItemUpdate : UpdatesList)
	{
		BroadcastItemUpdate(ItemUpdate);
	}
}

void UInventoryComponent::BroadcastInventoryUpdate()
{
	if (!OnUpdate.IsBound())
	{
		return;
	}

	OnUpdate.Broadcast();
}

void UInventoryComponent::BroadcastItemUpdate(const FInventoryItemUpdate& ItemUpdate)
{
	if (!OnItemUpdate.IsBound())
	{
		return;
	}

	OnItemUpdate.Broadcast(ItemUpdate);
}

bool UInventoryComponent::AddItemToSlots(const FInventoryItem& ItemToAdd, int32& Remaining, const int32& RowsToSkip)
{
	Remaining = ItemToAdd.Quantity;

	FItemData* ItemData = GetItemData(ItemToAdd.Name);
	if (ItemData == nullptr)
	{
		return false;
	}

	uint32 UniqueID = FMath::RandRange(1, 999999);
	
	// if the item stats are empty populate with defaults
	TArray<FItemStat> ItemStats;
	if (ItemData->Stats.Num() > 0 && ItemToAdd.Stats.Num() == 0)
	{
		ItemStats = ItemData->Stats;
	}
	else
	{
		ItemStats = ItemToAdd.Stats;
	}

	if (!FindAndAddToPopulatedSlot(ItemToAdd.Name, ItemData->StackSize, Remaining, RowsToSkip))
	{
		FindAndAddToEmptySlot(ItemToAdd.Name, ItemData->StackSize, ItemStats, Remaining, RowsToSkip);
	}

	return Remaining == 0;
}

bool UInventoryComponent::FindAndAddToPopulatedSlot(const FName& ItemName, const int32& ItemStackSize, int32& QuantityToAdd, const int32& RowsToSkip)
{
	AActor* OwningActor = GetOwner();
	if (OwningActor == nullptr)
	{
		return false;
	}

	const bool UseServerState = OwningActor->HasAuthority();

	TArray<FInventorySlot>& SlotArray = UseServerState ? ServerState.Slots : Slots;
	const int32 StartIndex = RowsToSkip * 6;
	for (int32 i = StartIndex; i < SlotArray.Num(); i++)
	{
		FInventorySlot& Slot = SlotArray[i];
		if (QuantityToAdd == 0)
		{
			break;
		}

		if (Slot.Item.Name != ItemName || Slot.Item.Quantity == 0 || Slot.Item.Quantity >= GetItemData(ItemName)->StackSize)
		{
			continue;
		}

		if (Slot.Item.Quantity + QuantityToAdd > ItemStackSize)
		{
			QuantityToAdd -= ItemStackSize - Slot.Item.Quantity;
			FInventoryItem NewSlotItem;
			NewSlotItem.Name = ItemName;
			NewSlotItem.Quantity = ItemStackSize;
			Slot.Item = NewSlotItem;
		}
		else
		{
			FInventoryItem NewSlotItem;
			NewSlotItem.Name = ItemName;
			NewSlotItem.Quantity = QuantityToAdd + Slot.Item.Quantity;
			Slot.Item = NewSlotItem;
			QuantityToAdd = 0;
		}
	}
	return QuantityToAdd == 0;
}

bool UInventoryComponent::FindAndAddToEmptySlot(const FName& ItemName, const int32& ItemStackSize, const TArray<FItemStat>& Stats, int32& QuantityToAdd, const int32& RowsToSkip)
{
	AActor* OwningActor = GetOwner();
	if (OwningActor == nullptr)
	{
		return false;
	}

	const bool UseServerState = OwningActor->HasAuthority();
	TArray<FInventorySlot>& SlotArray = UseServerState ? ServerState.Slots : Slots;
	const int32 StartIndex = RowsToSkip * 6;

	for (int32 i = StartIndex; i < SlotArray.Num(); i++)
	{
		FInventorySlot& Slot = SlotArray[i];

		if (QuantityToAdd == 0)
		{
			break;
		}

		if (Slot.Item.Quantity > 0)
		{
			continue;
		}

		uint32 ItemUniqueID = FMath::RandRange(0, 99999);

		if (QuantityToAdd > ItemStackSize)
		{
			QuantityToAdd -= ItemStackSize;
			FInventoryItem NewSlotItem;
			NewSlotItem.Name = ItemName;
			NewSlotItem.Quantity = ItemStackSize;
			NewSlotItem.UniqueID = ItemUniqueID;
			Slot.Item = NewSlotItem;
		}
		else
		{
			FInventoryItem NewSlotItem;
			NewSlotItem.Name = ItemName;
			NewSlotItem.Quantity = QuantityToAdd;
			NewSlotItem.Stats = Stats;
			NewSlotItem.UniqueID = ItemUniqueID;
			Slot.Item = NewSlotItem;
			QuantityToAdd = 0;
		}
	}
	return QuantityToAdd == 0;
}

bool UInventoryComponent::RemoveItemFromSlots(const FName& ItemName, const int32& Quantity, int32& Remaining)
{
	AActor* OwningActor = GetOwner();
	if (OwningActor == nullptr)
	{
		return false;
	}
	
	bool UseServerState = OwningActor->HasAuthority();

	Remaining = Quantity;
	TArray<FInventorySlot>& SlotArray = UseServerState ? ServerState.Slots : Slots;
	for (FInventorySlot& Slot : SlotArray)
	{
		if (Slot.Item.Name != ItemName)
		{
			continue;
		}

		if (Slot.Item.Quantity <= Remaining)
		{
			Remaining -= Slot.Item.Quantity;
			Slot.ClearItem();
			continue;
		}
		else
		{
			Slot.Item.Quantity -= Remaining;
			Remaining = 0;
		}
		
		if (Slot.Item.IsZero())
		{
			Slot.ClearItem();
		}

		if (Remaining == 0)
		{
			break;
		}
	}

	return Remaining == 0;
}

void UInventoryComponent::DragAll(const int32& FromSlotIndex, UInventoryManipulatorComponent* Manipulator, TArray<FInventorySlot>& SlotsToModify, FInventoryContents& ContentsToModify)
{
	FInventorySlot& FromSlot = SlotsToModify[FromSlotIndex];

	if (FromSlot.IsEmpty())
	{
		return;
	}

	Manipulator->StartDragging(FromSlot.Item);

	ContentsToModify.RemoveItem(FromSlot.Item.Name, FromSlot.Item.Quantity);

	FromSlot.ClearItem();
}

void UInventoryComponent::DragSplit(const int32& FromSlotIndex, UInventoryManipulatorComponent* Manipulator, TArray<FInventorySlot>& SlotsToModify, FInventoryContents& ContentsToModify)
{
	FInventorySlot& FromSlot = SlotsToModify[FromSlotIndex];

	if (FromSlot.IsEmpty())
	{
		return;
	}

	// Just take all if there is only one item
	if (FromSlot.Item.Quantity == 1)
	{
		Manipulator->StartDragging(FromSlot.Item);
		FromSlot.ClearItem();

		return;
	}

	// Calculate the half
	int32 HalfQuantity = FromSlot.Item.Quantity / 2;

	// Start Dragging half
	FInventoryItem NewSelection = FromSlot.Item;
	NewSelection.Quantity -= HalfQuantity;


	Manipulator->StartDragging(NewSelection);
	ContentsToModify.RemoveItem(NewSelection.Name, NewSelection.Quantity);

	FInventoryItem NewSlotItem = FromSlot.Item;
	NewSlotItem.Quantity = HalfQuantity;
	FromSlot.Item = NewSlotItem;
}

void UInventoryComponent::DropAll(const int32& ToSlotIndex, UInventoryManipulatorComponent* Manipulator, TArray<FInventorySlot>& SlotsToModify, FInventoryContents& ContentsToModify)
{
	FInventorySlot& ToSlot = SlotsToModify[ToSlotIndex];
	FInventoryItem SelectedItem = Manipulator->GetSelectedItem();

	if (ToSlot.IsEmpty())
	{
		ToSlot.Item = SelectedItem;

		ContentsToModify.AddItem(SelectedItem.Name, SelectedItem.Quantity);

		Manipulator->StopDragging();
	}
	else if (ToSlot.SameItemNameAs(SelectedItem) && GetItemData(SelectedItem.Name)->StackSize != 1) // Same item and the stack size for this item is not 1
	{
		if (WithinStackSize(ToSlot.Item, SelectedItem.Quantity))
		{
			// Update Inventory
			ToSlot.Item.Quantity += SelectedItem.Quantity;
			ContentsToModify.AddItem(Manipulator->GetSelectedItem().Name, Manipulator->GetSelectedItem().Quantity);
			
			// Update Selection
			Manipulator->StopDragging();
		}
		else
		{
			int32 OldSlotItemQuantity = ToSlot.Item.Quantity;

			// Update Inventory
			int32 AmountAdded = GetItemData(ToSlot.Item.Name)->StackSize - ToSlot.Item.Quantity;
			ToSlot.Item.Quantity = GetItemData(SelectedItem.Name)->StackSize;
			ContentsToModify.AddItem(SelectedItem.Name, AmountAdded);
			
			// Update Selection
			SelectedItem.Quantity = (OldSlotItemQuantity + SelectedItem.Quantity) - GetItemData(SelectedItem.Name)->StackSize;
			Manipulator->StartDragging(SelectedItem);
		}
	}
	else
	{
		// Store the old item for swap
		FInventoryItem OldSlotItem = ToSlot.Item;

		// Update Inventory
		ContentsToModify.RemoveItem(OldSlotItem.Name, OldSlotItem.Quantity);
		ContentsToModify.AddItem(SelectedItem.Name, SelectedItem.Quantity);
		ToSlot.Item = SelectedItem;

		// Update Selection
		Manipulator->StartDragging(OldSlotItem);
	}

	if (Manipulator->GetSelectedItem().IsZero())
	{
		Manipulator->StopDragging();
	}
}

void UInventoryComponent::DropSingle(const int32& ToSlotIndex, UInventoryManipulatorComponent* Manipulator, TArray<FInventorySlot>& SlotsToModify, FInventoryContents& ContentsToModify)
{
	if (!Manipulator->IsDragging())
	{
		return;
	}

	FInventorySlot& ToSlot = SlotsToModify[ToSlotIndex];
	FInventoryItem SelectedItem = Manipulator->GetSelectedItem();

	if (ToSlot.IsEmpty())
	{
		// Update Inventory
		ToSlot.Item.Name = SelectedItem.Name;
		ToSlot.Item.Quantity = 1;
		ContentsToModify.AddItem(SelectedItem.Name, 1);

		// Update Selection
		FInventoryItem NewSelection = SelectedItem;
		NewSelection.Quantity -= 1;
		Manipulator->StartDragging(NewSelection);
	}
	else if (ToSlot.SameItemNameAs(SelectedItem) && WithinStackSize(ToSlot.Item, 1))
	{
		// Update Inventory
		ToSlot.Item.Quantity += 1;
		ContentsToModify.AddItem(SelectedItem.Name, 1);

		// Update Selection
		FInventoryItem NewSelection = SelectedItem;
		NewSelection.Quantity -= 1;
		Manipulator->StartDragging(NewSelection);
	}

	if (Manipulator->GetSelectedItem().Quantity <= 0)
	{
		Manipulator->StopDragging();
	}
}

bool UInventoryComponent::WithinStackSize(const FInventoryItem& BaseItem, const int32& AmountToAdd) const
{
	return (BaseItem.Quantity + AmountToAdd) <= GetItemData(BaseItem.Name)->StackSize;
}