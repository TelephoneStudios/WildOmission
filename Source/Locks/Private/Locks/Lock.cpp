// Copyright Telephone Studios. All Rights Reserved.


#include "Locks/Lock.h"
#include "GameFramework/PlayerState.h"
#include "Components/LockComponent.h"
#include "Components/LockModifierComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ALock::ALock()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	MeshComponent = nullptr;
	SuccessSound = nullptr;
	FailureSound = nullptr;
	PlacementSound = nullptr;

	OwnerLockComponent = nullptr;

	Code = TEXT("");
	AuthorizedPlayers = TArray<FString>();
	Locked = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	static ConstructorHelpers::FObjectFinder<USoundBase> SuccessSoundObject(TEXT("/Game/Locks/Audio/A_CodeLock_Success"));
	if (SuccessSoundObject.Succeeded())
	{
		SuccessSound = SuccessSoundObject.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> FailureSoundObject(TEXT("/Game/Locks/Audio/A_CodeLock_Failure"));
	if (FailureSoundObject.Succeeded())
	{
		FailureSound = FailureSoundObject.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> PlacementSoundObject(TEXT("/Game/Deployables/Audio/Deployable_Placement_Cue"));
	if (PlacementSoundObject.Succeeded())
	{
		PlacementSound = PlacementSoundObject.Object;
	}
}

void ALock::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ALock, Locked);
}

void ALock::OnPlacement(ULockComponent* InOwnerLockComponent)
{
	OwnerLockComponent = InOwnerLockComponent;
	Multi_PlayPlacementSound();
}

void ALock::Interact(AActor* Interactor)
{
	if (Interactor == nullptr)
	{
		return;
	}

	ULockModifierComponent* InteractorLockModifierComponent = Interactor->FindComponentByClass<ULockModifierComponent>();
	if (InteractorLockModifierComponent == nullptr)
	{
		return;
	}

	InteractorLockModifierComponent->OpenKeypadMenu(this);
}

FString ALock::PromptText()
{
	return TEXT("open keypad");
}

void ALock::SetCode(const FString& NewCode, const FString& CodeSetterUniqueID)
{
	if (NewCode.Len() < 4)
	{
		return;
	}

	Code = NewCode;
	Locked = true;
	OnRep_Locked();
	if (!CodeSetterUniqueID.IsEmpty())
	{
		AuthorizedPlayers.Empty();
		AuthorizedPlayers.Add(CodeSetterUniqueID);
	}
}

void ALock::ClearCode()
{
	Code.Empty();
	Locked = false;
	OnRep_Locked();
}

FString ALock::GetCode() const
{
	return Code;
}

TArray<FString> ALock::GetAuthorizedPlayers() const
{
	return AuthorizedPlayers;
}

void ALock::SetAuthorizedPlayers(const TArray<FString>& InAuthorizedPlayers)
{
	AuthorizedPlayers = InAuthorizedPlayers;
}

bool ALock::IsLocked() const
{
	return Locked;
}

void ALock::AuthorizePlayer(const FString& PlayerUniqueID)
{
	AuthorizedPlayers.Add(PlayerUniqueID);
}

bool ALock::IsAuthorized(const FString& PlayerUniqueID) const
{
	for (const FString& AuthorizedPlayer : AuthorizedPlayers)
	{
		if (AuthorizedPlayer != PlayerUniqueID)
		{
			continue;
		}

		return true;
	}

	return false;
}

bool ALock::IsAuthorized(ULockModifierComponent* LockModifier) const
{
	if (LockModifier == nullptr)
	{
		return false;
	}

	APawn* PlayerPawn = Cast<APawn>(LockModifier->GetOwner());
	if (PlayerPawn == nullptr)
	{
		return false;
	}

	return IsAuthorized(PlayerPawn);
}

bool ALock::IsAuthorized(APawn* PlayerPawn) const
{
	if (PlayerPawn == nullptr)
	{
		return false;
	}

	APlayerState* OwnerPlayerState = PlayerPawn->GetPlayerState();
	if (OwnerPlayerState == nullptr)
	{
		return false;
	}

	return IsAuthorized(OwnerPlayerState->GetUniqueId()->ToString());
}

UStaticMesh* ALock::GetStaticMesh() const
{
	return MeshComponent->GetStaticMesh();
}

ULockComponent* ALock::GetOwnerLockComponent() const
{
	return OwnerLockComponent;
}

void ALock::OnRep_Locked()
{
	MeshComponent->SetDefaultCustomPrimitiveDataFloat(0, static_cast<float>(Locked));
}

void ALock::Multi_PlaySuccessSound_Implementation()
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), SuccessSound, this->GetActorLocation());
}

void ALock::Multi_PlayFailureSound_Implementation()
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), FailureSound, this->GetActorLocation());
}

void ALock::Multi_PlayPlacementSound_Implementation()
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), PlacementSound, this->GetActorLocation());
}