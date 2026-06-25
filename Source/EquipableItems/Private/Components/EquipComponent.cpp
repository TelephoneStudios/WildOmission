// Copyright Telephone Studios. All Rights Reserved.


#include "Components/EquipComponent.h"
#include "Components/PlayerInventoryComponent.h"
#include "Items/EquipableItem.h"
#include "Items/ToolItem.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Log.h"

// Sets default values for this component's properties
UEquipComponent::UEquipComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	SetIsReplicatedByDefault(true);

	FirstPersonItemComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonItemComponent"));
	FirstPersonItemComponent->SetAnimInstanceClass(UAnimInstance::StaticClass());
	FirstPersonItemComponent->SetCastShadow(false);
	
	PrimaryHeld = false;
	SecondaryHeld = false;
	EquipedItem = nullptr;
	LocalEquipedItemDefaultClass = nullptr;

	OwnerPawn = nullptr;
	OwnerFirstPersonMesh = nullptr;
	OwnerThirdPersonMesh = nullptr;

	HitmarkerSound = nullptr;
	HeadshotHitmarkerSound = nullptr;

	static ConstructorHelpers::FObjectFinder<USoundBase> HitmarkerSoundObject(TEXT("/Game/Weapons/Audio/A_Hitmarker_Body_02"));
	if (HitmarkerSoundObject.Succeeded())
	{
		HitmarkerSound = HitmarkerSoundObject.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> HeadshotHitmarkerSoundObject(TEXT("/Game/Weapons/Audio/A_Hitmarker_Head"));
	if (HeadshotHitmarkerSoundObject.Succeeded())
	{
		HeadshotHitmarkerSound = HeadshotHitmarkerSoundObject.Object;
	}
}

void UEquipComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UEquipComponent, EquipedItem);
}

void UEquipComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsEquipedItemValid() && PrimaryHeld)
	{
		EquipedItem->OnPrimaryHeld();
	}

	if (IsEquipedItemValid()  && SecondaryHeld)
	{
		EquipedItem->OnSecondaryHeld();
	}
}

void UEquipComponent::Setup(USkeletalMeshComponent* FirstPersonMeshComponent, USkeletalMeshComponent* ThirdPersonMeshComponent)
{
	OwnerFirstPersonMesh = FirstPersonMeshComponent;
	OwnerThirdPersonMesh = ThirdPersonMeshComponent;
	
	if (FirstPersonItemComponent == nullptr)
	{
		return;
	}

	FirstPersonItemComponent->AttachToComponent(FirstPersonMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("RightHandMountSocket"));
}

void UEquipComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr)
	{
		return;
	}

	PrimaryHeld = false;
	SecondaryHeld = false;

	UPlayerInventoryComponent* OwnerInventory = OwnerPawn->FindComponentByClass<UPlayerInventoryComponent>();
	if (OwnerInventory == nullptr)
	{
		return;
	}

	OwnerInventory->OnToolbarSlotSelectionChange.AddDynamic(this, &UEquipComponent::RefreshEquip);
}

void UEquipComponent::EquipItem(const FName& ItemName, TSubclassOf<AEquipableItem> ItemClass, const int8& FromSlotIndex, const uint32& UniqueID)
{
	UWorld* World = GetWorld();
	if (World == nullptr || OwnerPawn == nullptr || ItemClass == nullptr)
	{
		return;
	}

	if (OwnerPawn->IsLocallyControlled() && OwnerPawn->IsPlayerControlled())
	{
		EquipFirstPersonViewModel(ItemClass, UniqueID);
	}

	if (OwnerPawn->HasAuthority())
	{	
		EquipedItem = World->SpawnActor<AEquipableItem>(ItemClass, OwnerPawn->GetActorLocation(), OwnerPawn->GetActorRotation());
		if (EquipedItem == nullptr)
		{
			return;
		}

		EquipedItem->Equip(OwnerPawn, OwnerThirdPersonMesh, ItemName, FromSlotIndex, UniqueID);
		OnRep_EquipedItem();
	}
}

void UEquipComponent::Disarm()
{
	if (OwnerPawn == nullptr)
	{
		return;
	}

	if (OwnerPawn && OwnerPawn->IsLocallyControlled() && OwnerPawn->IsPlayerControlled())
	{
		EquipFirstPersonViewModel(nullptr, 0);
	}

	if (OwnerPawn->HasAuthority() && IsItemEquiped())
	{
		EquipedItem->OnUnequip();
		DestroyEquipedItem();

		OnRep_EquipedItem();
	}

	StopAiming();
	StopAllItemMontages();
}

void UEquipComponent::DestroyEquipedItem()
{
	if (EquipedItem == nullptr)
	{
		return;
	}

	EquipedItem->UponDestruction();
	EquipedItem->Destroy();
	EquipedItem = nullptr;
}

void UEquipComponent::UpdateControlRotation(const FRotator& NewControlRotation)
{
	OwnerReplicatedControlRotation = NewControlRotation;
}

FRotator UEquipComponent::GetOwnerControlRotation() const
{
	if (OwnerPawn && OwnerReplicatedControlRotation == FRotator::ZeroRotator)
	{
		return OwnerPawn->GetActorRotation();
	}

	return OwnerReplicatedControlRotation;
}

FVector UEquipComponent::GetOwnerVelocity() const
{
	if (OwnerPawn == nullptr)
	{
		return FVector::ZeroVector;
	}

	return OwnerPawn->GetVelocity();
}

USkeletalMeshComponent* UEquipComponent::GetFirstPersonItemComponent() const
{
	return FirstPersonItemComponent;
}

void UEquipComponent::SetArmLocationOffset(const FVector& InOffset)
{
	if (OwnerFirstPersonMesh == nullptr)
	{
		return;
	}
	
	const FVector DefaultArmLocation = FVector(-5.0f, 0.0f, -160.0f);
	OwnerFirstPersonMesh->SetRelativeLocation(DefaultArmLocation + InOffset);
}

void UEquipComponent::SetArmRotationOffset(const FRotator& InOffset)
{
	if (OwnerFirstPersonMesh == nullptr)
	{
		return;
	}

	OwnerFirstPersonMesh->SetRelativeRotation(InOffset);
}

FVector UEquipComponent::GetCurrentArmLocationOffset() const
{
	if (OwnerFirstPersonMesh == nullptr)
	{
		return FVector::ZeroVector;
	}

	const FVector DefaultArmLocation = FVector(-5.0f, 0.0f, -160.0f);
	return OwnerFirstPersonMesh->GetRelativeLocation() - DefaultArmLocation;
}

FRotator UEquipComponent::GetCurrentArmRotationOffset() const
{
	if (OwnerFirstPersonMesh == nullptr)
	{
		return FRotator::ZeroRotator;
	}

	return OwnerFirstPersonMesh->GetRelativeRotation();
}

void UEquipComponent::PlayItemMontage(UAnimMontage* PlayerMontage, UAnimMontage* ItemMontage)
{
	if (OwnerPawn == nullptr)
	{
		return;
	}

	const bool UseFirstPersonInstance = OwnerPawn->IsLocallyControlled() && OwnerPawn->IsPlayerControlled();
	
	if (UseFirstPersonInstance && LocalEquipedItemDefaultClass)
	{
		UAnimInstance* FirstPersonAnimInstance = OwnerFirstPersonMesh->GetAnimInstance();
		UAnimInstance* FirstPersonItemAnimInstance = FirstPersonItemComponent->GetAnimInstance();

		if (FirstPersonAnimInstance && PlayerMontage)
		{
			FirstPersonAnimInstance->Montage_Play(PlayerMontage);
		}
		if (FirstPersonItemAnimInstance && ItemMontage)
		{
			FirstPersonItemAnimInstance->Montage_Play(ItemMontage);
		}
	}
	else if (!UseFirstPersonInstance && EquipedItem)
	{
		UAnimInstance* ThirdPersonAnimInstance = OwnerThirdPersonMesh->GetAnimInstance();
		UAnimInstance* ThirdPersonItemAnimInstance = EquipedItem->GetMeshComponent()->GetAnimInstance();

		if (ThirdPersonAnimInstance && PlayerMontage)
		{
			ThirdPersonAnimInstance->Montage_Play(PlayerMontage);
		}
		if (ThirdPersonItemAnimInstance && ItemMontage)
		{
			ThirdPersonItemAnimInstance->Montage_Play(ItemMontage);
		}
	}

	if (OwnerPawn->HasAuthority())
	{
		Multi_PlayItemMontage(PlayerMontage, ItemMontage);
	}
}

void UEquipComponent::StopAllItemMontages()
{
	if (OwnerPawn == nullptr)
	{
		return;
	}

	const float BlendOutTimeSeconds = 0.1f;
	const bool UseFirstPersonInstance = OwnerPawn->IsLocallyControlled() && OwnerPawn->IsPlayerControlled();

	if (UseFirstPersonInstance && OwnerFirstPersonMesh && FirstPersonItemComponent)
	{
		UAnimInstance* FirstPersonAnimInstance = OwnerFirstPersonMesh->GetAnimInstance();
		UAnimInstance* FirstPersonItemAnimInstance = FirstPersonItemComponent->GetAnimInstance();
		if (FirstPersonAnimInstance == nullptr || FirstPersonItemAnimInstance == nullptr)
		{
			return;
		}

		FirstPersonAnimInstance->StopAllMontages(BlendOutTimeSeconds);
		FirstPersonItemAnimInstance->StopAllMontages(BlendOutTimeSeconds);
	}
	else if (!UseFirstPersonInstance && OwnerThirdPersonMesh && EquipedItem)
	{
		UAnimInstance* ThirdPersonAnimInstance = OwnerThirdPersonMesh->GetAnimInstance();
		UAnimInstance* ThirdPersonItemAnimInstance = EquipedItem->GetMeshComponent()->GetAnimInstance();
		if (ThirdPersonAnimInstance == nullptr || ThirdPersonItemAnimInstance == nullptr)
		{
			return;
		}

		ThirdPersonAnimInstance->StopAllMontages(BlendOutTimeSeconds);
		ThirdPersonItemAnimInstance->StopAllMontages(BlendOutTimeSeconds);
	}

	if (OwnerPawn->HasAuthority())
	{
		Multi_StopAllItemMontages();
	}
}

bool UEquipComponent::IsMontagePlaying(UAnimMontage* Montage) const
{
	return IsFirstPersonMontagePlaying(Montage) || IsThirdPersonMontagePlaying(Montage);
}

bool UEquipComponent::IsItemMontagePlaying(UAnimMontage* Montage) const
{
	UAnimInstance* FirstPersonItemAnimInstance = FirstPersonItemComponent->GetAnimInstance();
	if (FirstPersonItemAnimInstance == nullptr || EquipedItem == nullptr)
	{
		return false;
	}

	UAnimInstance* ThirdPersonItemAnimInstance = EquipedItem->GetMeshComponent()->GetAnimInstance();
	if (ThirdPersonItemAnimInstance == nullptr)
	{
		return false;
	}

	return FirstPersonItemAnimInstance->Montage_IsPlaying(Montage) || ThirdPersonItemAnimInstance->Montage_IsPlaying(Montage);
}

void UEquipComponent::OnPrimaryAnimationClimax(bool FirstPerson)
{
	if (EquipedItem == nullptr)
	{
		return;
	}

	EquipedItem->OnPrimaryAnimationClimax(FirstPerson);
}

void UEquipComponent::OnSecondaryAnimationClimax(bool FirstPerson)
{
	if (EquipedItem == nullptr)
	{
		return;
	}

	EquipedItem->OnSecondaryAnimationClimax(FirstPerson);
}

void UEquipComponent::OnReloadAnimationClimax(bool FirstPerson)
{
	if (EquipedItem == nullptr)
	{
		return;
	}

	EquipedItem->OnReloadAnimationClimax(FirstPerson);
}

void UEquipComponent::OnChamberedAnimationClimax(bool FirstPerson)
{
	if (EquipedItem == nullptr)
	{
		return;
	}

	EquipedItem->OnChamberedAnimationClimax(FirstPerson);
}

AEquipableItem* UEquipComponent::GetEquipedItem() const
{
	return EquipedItem;
}

UAnimSequence* UEquipComponent::GetEquipedItemPose() const
{
	if (OwnerPawn == nullptr)
	{
		return nullptr;
	}

	if (OwnerPawn->IsLocallyControlled() &&	OwnerPawn->IsPlayerControlled())
	{
		if (EquipedItem)
		{
			return EquipedItem->GetEquipPose();
		}
		else if (LocalEquipedItemDefaultClass)
		{
			return LocalEquipedItemDefaultClass->GetEquipPose();
		}
	}
	else if (EquipedItem)
	{
		return EquipedItem->GetEquipPose();
	}

	return nullptr;
}

FRotator UEquipComponent::GetEquipedItemRightArmOffset() const
{
	if (OwnerPawn == nullptr)
	{
		return FRotator::ZeroRotator;
	}

	if (OwnerPawn->IsLocallyControlled() && OwnerPawn->IsPlayerControlled())
	{
		if (EquipedItem)
		{
			return EquipedItem->GetRightArmVelocityOffset();
		}
		else if (LocalEquipedItemDefaultClass)
		{
			return LocalEquipedItemDefaultClass->GetRightArmVelocityOffset();
		}
	}
	else if (EquipedItem)
	{
		return EquipedItem->GetRightArmVelocityOffset();
	}

	return FRotator::ZeroRotator;
}

FRotator UEquipComponent::GetEquipedItemLeftArmOffset() const
{
	if (OwnerPawn == nullptr)
	{
		return FRotator::ZeroRotator;
	}

	if (OwnerPawn->IsLocallyControlled() && OwnerPawn->IsPlayerControlled())
	{
		if (EquipedItem)
		{
			return EquipedItem->GetRightArmVelocityOffset();
		}
		else if (LocalEquipedItemDefaultClass)
		{
			return LocalEquipedItemDefaultClass->GetRightArmVelocityOffset();
		}
	}
	else if (EquipedItem)
	{
		return EquipedItem->GetRightArmVelocityOffset();
	}

	return FRotator::ZeroRotator;
}

bool UEquipComponent::IsItemEquiped() const
{
	return EquipedItem != nullptr;
}

bool UEquipComponent::PrimaryEnabled() const
{
	return IsItemEquiped() && EquipedItem->PrimaryEnabled();
}

bool UEquipComponent::SecondaryEnabled() const
{
	return IsItemEquiped() && EquipedItem->SecondaryEnabled();
}

void UEquipComponent::PrimaryPressed()
{
	if (!GetOwner()->HasAuthority())
	{
		Server_PrimaryPressed();
	}

	PrimaryHeld = true;

	if (!IsEquipedItemValid())
	{
		return;
	}

	EquipedItem->OnPrimaryPressed();
}

void UEquipComponent::PrimaryReleased()
{
	if (!GetOwner()->HasAuthority())
	{
		Server_PrimaryReleased();
	}

	if (PrimaryHeld == false)
	{
		return;
	}

	PrimaryHeld = false;

	if (!IsEquipedItemValid())
	{
		return;
	}

	EquipedItem->OnPrimaryReleased();
}

void UEquipComponent::SecondaryPressed()
{
	if (!GetOwner()->HasAuthority())
	{
		Server_SecondaryPressed();
	}

	SecondaryHeld = true;

	if (!IsEquipedItemValid())
	{
		return;
	}

	EquipedItem->OnSecondaryPressed();
}

void UEquipComponent::SecondaryReleased()
{
	if (!GetOwner()->HasAuthority())
	{
		Server_SecondaryReleased();
	}

	if (SecondaryHeld == false)
	{
		return;
	}

	SecondaryHeld = false;
	
	if (!IsEquipedItemValid())
	{
		return;
	}

	EquipedItem->OnSecondaryReleased();
}

void UEquipComponent::ReloadPressed()
{
	if (!GetOwner()->HasAuthority())
	{
		Server_ReloadPressed();
	}

	if (!IsEquipedItemValid() || PrimaryHeld || SecondaryHeld)
	{
		return;
	}

	EquipedItem->OnReloadPressed();
}

void UEquipComponent::StartAiming() const
{
	if (!OnStartAiming.IsBound())
	{
		return;
	}

	OnStartAiming.Broadcast();
}

void UEquipComponent::StopAiming() const
{
	if (!OnStopAiming.IsBound())
	{
		return;
	}

	OnStopAiming.Broadcast();
}

void UEquipComponent::HandleHitmarker(bool IsHeadshot)
{
	if (OnHitmarker.IsBound())
	{
		OnHitmarker.Broadcast(IsHeadshot);
	}
	
	Client_PlayHitmarkerSound(IsHeadshot);
}

void UEquipComponent::Client_PlayHitmarkerSound_Implementation(bool IsHeadshot)
{
	
	IsHeadshot ? UGameplayStatics::PlaySound2D(GetWorld(), HeadshotHitmarkerSound) : UGameplayStatics::PlaySound2D(GetWorld(), HitmarkerSound);
}

void UEquipComponent::OnRep_EquipedItem()
{
	if (OwnerPawn == nullptr)
	{
		return;
	}

	if (OnItemEquiped.IsBound())
	{
		OnItemEquiped.Broadcast(EquipedItem);
	}

	if (EquipedItem == nullptr)
	{
		return;
	}

	RefreshEquipedSlotUI.Broadcast(EquipedItem->GetFromSlotIndex());

	EquipedItem->SetLocalVisibility(!OwnerPawn->IsLocallyControlled());

	if (OwnerPawn->IsLocallyControlled() && OwnerPawn->IsPlayerControlled())
	{
		return;
	}

	PlayItemMontage(EquipedItem->GetEquipMontage(), EquipedItem->GetEquipItemMontage());
}

void UEquipComponent::EquipFirstPersonViewModel(TSubclassOf<AEquipableItem> ItemClass, const uint32& UniqueID)
{
	if (OwnerPawn == nullptr || FirstPersonItemComponent == nullptr)
	{
		return;
	}

	if (ItemClass == nullptr)
	{
		LocalEquipedItemDefaultClass = nullptr;
		FirstPersonItemComponent->SetVisibility(false);
		return;
	}

	LocalEquipedItemDefaultClass = ItemClass.GetDefaultObject();
	if (LocalEquipedItemDefaultClass == nullptr || OwnerFirstPersonMesh == nullptr)
	{
		return;
	}

	LocalEquipedItemDefaultClass->SetUniqueItemID(UniqueID);

	LocalEquipedItemDefaultClass->IsLeftHandMounted() ?
		FirstPersonItemComponent->AttachToComponent(OwnerFirstPersonMesh, FAttachmentTransformRules::KeepRelativeTransform, TEXT("LeftHandMountSocket")) :
		FirstPersonItemComponent->AttachToComponent(OwnerFirstPersonMesh, FAttachmentTransformRules::KeepRelativeTransform, TEXT("RightHandMountSocket"));

	FirstPersonItemComponent->SetSkeletalMeshAsset(LocalEquipedItemDefaultClass->GetMeshComponent()->GetSkeletalMeshAsset());
	FirstPersonItemComponent->SetAnimInstanceClass(LocalEquipedItemDefaultClass->GetMeshComponent()->GetAnimClass());

	FirstPersonItemComponent->SetVisibility(OwnerPawn->IsLocallyControlled());
	FirstPersonItemComponent->SetRelativeLocation(LocalEquipedItemDefaultClass->GetSocketOffset().GetLocation());
	FirstPersonItemComponent->SetRelativeRotation(LocalEquipedItemDefaultClass->GetSocketOffset().GetRotation());
	FirstPersonItemComponent->SetRelativeScale3D(LocalEquipedItemDefaultClass->GetSocketOffset().GetScale3D());
	
	PlayItemMontage(LocalEquipedItemDefaultClass->GetEquipMontage(), LocalEquipedItemDefaultClass->GetEquipItemMontage());
}

void UEquipComponent::RefreshEquip(const int8& NewSlotIndex, const FInventorySlot& NewSlot)
{
	if (OwnerPawn == nullptr)
	{
		Disarm();
		return;
	}

	// return if there is no item
	if (NewSlot.IsEmpty())
	{
		Disarm();
		return;
	}

	// get the equipable subclass for this item
	FItemData* SlotItemData = UInventoryComponent::GetItemData(NewSlot.Item.Name);
	if (SlotItemData == nullptr || SlotItemData->EquipItemClass == nullptr)
	{
		Disarm();
		return;
	}

	AEquipableItem* CurrentEquipedItem = EquipedItem;
	AEquipableItem* CurrentEquipedDefaultClass = LocalEquipedItemDefaultClass;

	// is this item the same as we are already holding
	if (GetOwner()->HasAuthority() && CurrentEquipedItem && SlotItemData->EquipItemClass.Get() == CurrentEquipedItem->GetClass() && NewSlot.Item.UniqueID == CurrentEquipedItem->GetUniqueItemID())
	{
		return;
	}

	// Locallized version of same item check
	if (OwnerPawn->IsLocallyControlled() && CurrentEquipedDefaultClass && SlotItemData->EquipItemClass.Get() == CurrentEquipedDefaultClass->GetClass() && NewSlot.Item.UniqueID == CurrentEquipedDefaultClass->GetUniqueItemID())
	{
		return;
	}

	Disarm();
	EquipItem(NewSlot.Item.Name, SlotItemData->EquipItemClass.Get(), NewSlotIndex, NewSlot.Item.UniqueID);

}

bool UEquipComponent::IsEquipedItemValid() const
{
	if (OwnerPawn && OwnerPawn->IsLocallyControlled() && OwnerPawn->IsPlayerControlled())
	{
		return EquipedItem != nullptr && LocalEquipedItemDefaultClass != nullptr && EquipedItem->GetClass() == LocalEquipedItemDefaultClass->GetClass();
	}

	return EquipedItem != nullptr;
}

bool UEquipComponent::IsFirstPersonMontagePlaying(UAnimMontage* Montage) const
{
	if (OwnerFirstPersonMesh == nullptr)
	{
		return false;
	}

	UAnimInstance* FirstPersonArmsAnimInstance = OwnerFirstPersonMesh->GetAnimInstance();
	if (FirstPersonArmsAnimInstance == nullptr)
	{
		return false;
	}

	return FirstPersonArmsAnimInstance->Montage_IsPlaying(Montage);
}

bool UEquipComponent::IsThirdPersonMontagePlaying(UAnimMontage* Montage) const
{
	if (OwnerThirdPersonMesh == nullptr)
	{
		return false;
	}
	
	UAnimInstance* ThirdPersonAnimInstance = OwnerThirdPersonMesh->GetAnimInstance();
	if (ThirdPersonAnimInstance == nullptr)
	{
		return false;
	}

	return ThirdPersonAnimInstance->Montage_IsPlaying(Montage);
}

//******************************************
//	RPC
//******************************************

void UEquipComponent::Server_PrimaryPressed_Implementation()
{
	PrimaryPressed();
}

void UEquipComponent::Server_PrimaryReleased_Implementation()
{
	PrimaryReleased();
}

void UEquipComponent::Server_SecondaryPressed_Implementation()
{
	SecondaryPressed();
}

void UEquipComponent::Server_SecondaryReleased_Implementation()
{
	SecondaryReleased();
}

void UEquipComponent::Server_ReloadPressed_Implementation()
{
	ReloadPressed();
}

bool UEquipComponent::MulticastMontageConditionsValid() const
{
	if (OwnerPawn == nullptr)
	{
		return false;
	}

	if (OwnerPawn->IsLocallyControlled() && OwnerPawn->IsPlayerControlled())
	{
		return false;
	}

	if (OwnerPawn->HasAuthority())
	{
		return false;
	}

	return true;
}

void UEquipComponent::Multi_PlayItemMontage_Implementation(UAnimMontage* PlayerMontage, UAnimMontage* ItemMontage)
{
	if (!MulticastMontageConditionsValid())
	{
		return;
	}

	PlayItemMontage(PlayerMontage, ItemMontage);
}

void UEquipComponent::Multi_StopAllItemMontages_Implementation()
{
	if (!MulticastMontageConditionsValid())
	{
		return;
	}

	StopAllItemMontages();
}