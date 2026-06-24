// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Structs/InventorySlotInteraction.h"
#include "Structs/InventoryItemUpdate.h"
#include "Structs/InventoryState.h"
#include "Structs/ItemData.h"
#include "Interfaces/SavableObject.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInventoryUpdateSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryItemUpdateSignature, const FInventoryItemUpdate&, ItemUpdate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemHarvestedSignature, const FInventoryItem&, ItemHarvested);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INVENTORY_API UInventoryComponent : public UActorComponent, public ISavableObject
{
	GENERATED_BODY()

public:	
	UInventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable)
	void AddItem(const FInventoryItem& ItemToAdd, bool FromHarvesting = false, AActor* ActorToSpawnDropedItems = nullptr, bool ForceClientUpdate = false);

	UFUNCTION(BlueprintCallable)
	void RemoveItem(const FInventoryItem& ItemToRemove);	

	void SlotInteraction(const int32& SlotIndex, UInventoryManipulatorComponent* Manipulator, bool QuickMove, bool Primary = true);
	
	static FItemData* GetItemData(const FName& ItemID);
	static void SpawnWorldItem(UWorld* WorldContextObject, const struct FInventoryItem& ItemToSpawn, AActor* SpawningActor);

	static TArray<FName> GetAllItems();

	FInventoryUpdateSignature OnUpdate;
	FOnInventoryItemUpdateSignature OnItemUpdate;
	FOnItemHarvestedSignature OnItemHarvested;

	FInventoryItem* FindItemWithUniqueID(const uint32& UniqueID);
	FInventorySlot* FindSlotContainingItem(const FName& ItemToFind);	

	FString GetDisplayName() const;
	UClass* GetWidgetClass() const;
	FInventoryContents* GetContents();
	FInventorySlot* GetSlot(const int32& SlotIndex);
	uint8 GetSlotCount() const;
	TArray<FInventorySlot>& GetSlots();

	void RequestInventoryRefresh();
	
	void SetDisplayName(const FString& NewDisplayName);
	void SetSlotCount(const uint8& InSlotCount);

	UFUNCTION(BlueprintCallable)
	bool WasLoadedFromSave() const;

	TArray<uint8> Save();	
	void Load(const TArray<uint8>& InSave);

protected:

	UPROPERTY(EditDefaultsOnly)
	uint8 SlotCount;

	UPROPERTY(VisibleAnywhere, SaveGame)
	FInventoryContents Contents;

	UPROPERTY(VisibleAnywhere, SaveGame)
	TArray<FInventorySlot> Slots;

	UPROPERTY(VisibleAnywhere, Replicated, ReplicatedUsing = OnRep_ServerState)
	FInventoryState ServerState;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UUserWidget> WidgetClass;

	UFUNCTION()
	virtual void OnRep_ServerState();

	UFUNCTION()
	void BroadcastInventoryUpdate();

	UFUNCTION()
	void BroadcastItemUpdate(const FInventoryItemUpdate& ItemUpdate);

private:
	bool AddItemToSlots(const FInventoryItem& ItemToAdd, int32& Remaining, const int32& RowsToSkip = 0);
	bool FindAndAddToPopulatedSlot(const FName& ItemName, const int32& ItemStackSize, int32& QuantityToAdd, const int32& RowsToSkip = 0);
	bool FindAndAddToEmptySlot(const FName& ItemName, const int32& ItemStackSize, const TArray<FItemStat>& Stats, int32& QuantityToAdd, const int32& RowsToSkip = 0);
	bool RemoveItemFromSlots(const FName& ItemName, const int32& Quantity, int32& Remaining);
	
	void DragAll(const int32& FromSlotIndex, UInventoryManipulatorComponent* Manipulator, TArray<FInventorySlot>& SlotsToModify, FInventoryContents& ContentsToModify);
	void DragSplit(const int32& FromSlotIndex, UInventoryManipulatorComponent* Manipulator, TArray<FInventorySlot>& SlotsToModify, FInventoryContents& ContentsToModify);
	void DropAll(const int32& ToSlotIndex, UInventoryManipulatorComponent* Manipulator, TArray<FInventorySlot>& SlotsToModify, FInventoryContents& ContentsToModify);
	void DropSingle(const int32& ToSlotIndex, UInventoryManipulatorComponent* Manipulator, TArray<FInventorySlot>& SlotsToModify, FInventoryContents& ContentsToModify);

	bool WithinStackSize(const FInventoryItem& Item, const int32& AmountToAdd) const;

	TArray<FInventorySlotInteraction> UnacknowalgedInteractions;

	FInventoryItemUpdate LastClientAcknowlagedItemUpdate;

	UPROPERTY(EditDefaultsOnly)
	FString DisplayName;

	void ClearAcknowlagedInteractions(const FInventorySlotInteraction& LastInteraction);
	void GetUnacknowlagedUpdates(TArray<FInventoryItemUpdate>& OutUpdatesList);

	void BroadcastAllUpdateNotifications(const TArray<FInventoryItemUpdate>& UpdatesList);

	bool LoadedFromSave;

	UFUNCTION()
	virtual void OnLoadComplete_Implementation() override;

	UFUNCTION(Server, Reliable)
	void Server_SlotInteraction(const FInventorySlotInteraction& Interaction);

	void HandleItemSelection(const FInventorySlotInteraction& Interaction);
	void HandleItemQuickMove(const FInventorySlotInteraction& Interaction);

};
