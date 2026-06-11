// Copyright Telephone Studios. All Rights Reserved.


#include "Components/LockModifierComponent.h"
#include "Locks/Lock.h"
#include "Components/LockComponent.h"
#include "GameFramework/PlayerState.h"
#include "UI/KeypadWidget.h"
#include "Components/InventoryComponent.h"
#include "Log.h"

// Sets default values for this component's properties
ULockModifierComponent::ULockModifierComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	KeypadWidget = nullptr;
	CurrentLock = nullptr;

	static ConstructorHelpers::FClassFinder<UKeypadWidget> KeypadWidgetBlueprint(TEXT("/Game/Locks/UI/WBP_KeypadMenu"));
	if (KeypadWidgetBlueprint.Succeeded())
	{
		KeypadWidgetClass = KeypadWidgetBlueprint.Class;
	}
}

void ULockModifierComponent::OpenKeypadMenu(ALock* Lock)
{
	if (!IsValid(Lock))
	{
		return;
	}

	TEnumAsByte<ELockOperation> LockOperation;

	if (Lock->IsAuthorized(GetOwnerUniqueID()) && Lock->IsLocked())
	{
		LockOperation = ELockOperation::ELO_ModifyCode;
	}
	else if (!Lock->IsAuthorized(GetOwnerUniqueID()) && Lock->IsLocked())
	{
		LockOperation = ELockOperation::ELO_Authorize;
	}
	else if (!Lock->IsLocked())
	{
		LockOperation = ELockOperation::ELO_SetCode;
	}

	Client_OpenKeypadMenu(Lock, LockOperation.GetIntValue());
}

bool ULockModifierComponent::Server_SetLockCode_Validate(ALock* Lock, const FString& Code)
{
	if (!IsValid(Lock) || (Lock->IsLocked() && !Lock->IsAuthorized(GetOwnerUniqueID())))
	{
		return false;
	}
	return true;
}

void ULockModifierComponent::Server_SetLockCode_Implementation(ALock* Lock, const FString& Code)
{
	if (!IsValid(Lock) || Code.Len() < 4)
	{
		return;
	}

	Lock->SetCode(Code, GetOwnerUniqueID());
	Lock->Multi_PlaySuccessSound();
}

bool ULockModifierComponent::Server_ClearLockCode_Validate(ALock* Lock)
{
	if (!IsValid(Lock) || !Lock->IsAuthorized(GetOwnerUniqueID()))
	{
		return false;
	}
	return true;
}

void ULockModifierComponent::Server_ClearLockCode_Implementation(ALock* Lock)
{
	if (!IsValid(Lock))
	{
		return;
	}

	Lock->ClearCode();
}

void ULockModifierComponent::Server_AuthorizeLock_Implementation(ALock* Lock, const FString& Code)
{
	if (!IsValid(Lock))
	{
		return;
	}

	if (Lock->GetCode() != Code)
	{
		Lock->Multi_PlayFailureSound();
		return;
	}

	Lock->AuthorizePlayer(GetOwnerUniqueID());
	Lock->Multi_PlaySuccessSound();
}

bool ULockModifierComponent::Server_RemoveLock_Validate(ALock* Lock)
{
	if (!IsValid(Lock) || (Lock->IsLocked() && !Lock->IsAuthorized(GetOwnerUniqueID())))
	{
		return false;
	}

	return true;
}

void ULockModifierComponent::Server_RemoveLock_Implementation(ALock* Lock)
{
	if (!IsValid(Lock))
	{
		return;
	}
	
	ULockComponent* OwnerLockComponent = Lock->GetOwnerLockComponent();
	if (OwnerLockComponent == nullptr)
	{
		return;
	}

	OwnerLockComponent->RemoveLock();

	AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr)
	{
		return;
	}

	UInventoryComponent* OwnerInventoryComponent = OwnerActor->FindComponentByClass<UInventoryComponent>();
	if (OwnerInventoryComponent == nullptr)
	{
		return;
	}

	FInventoryItem CodeLockItem;
	CodeLockItem.Name = TEXT("lock.code");
	CodeLockItem.Quantity = 1;

	OwnerInventoryComponent->AddItem(CodeLockItem);
}

FString ULockModifierComponent::GetOwnerUniqueID() const
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr)
	{
		return TEXT("");
	}

	AController* OwnerController = OwnerPawn->GetController();
	if (OwnerController == nullptr)
	{
		return TEXT("");
	}

	APlayerState* OwnerPlayerState = OwnerController->GetPlayerState<APlayerState>();
	if (OwnerPlayerState == nullptr)
	{
		return TEXT("");
	}

	return OwnerPlayerState->GetUniqueId()->ToString();
}

void ULockModifierComponent::Client_OpenKeypadMenu_Implementation(ALock* Lock, const uint8& LockOperation)
{
	UWorld* World = GetWorld();
	if (World == nullptr || KeypadWidget || Lock == nullptr || KeypadWidgetClass == nullptr)
	{
		return;
	}
	CurrentLock = Lock;
	KeypadWidget = CreateWidget<UKeypadWidget>(World, KeypadWidgetClass);
	KeypadWidget->OnTeardown.AddDynamic(this, &ULockModifierComponent::OnKeypadTeardown);
	
	const TEnumAsByte<ELockOperation> ActualLockOperation = TEnumAsByte<ELockOperation>(LockOperation);
	KeypadWidget->Setup(Lock, this, ActualLockOperation);
}

void ULockModifierComponent::OnKeypadTeardown()
{
	CurrentLock = nullptr;
	KeypadWidget = nullptr;
}