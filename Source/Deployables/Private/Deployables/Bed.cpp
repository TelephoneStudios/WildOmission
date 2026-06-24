// Copyright Telephone Studios. All Rights Reserved.


#include "Deployables/Bed.h"
#include "Interfaces/BedController.h"
#include "TimeOfDayManager.h"
#include "Net/UnrealNetwork.h"

ABed::ABed()
{
	SpawnPointComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPointComponent"));
	SpawnPointComponent->SetupAttachment(MeshComponent);

	UniqueID = -1;
}

void ABed::OnSpawn()
{
	Super::OnSpawn();

	UniqueID = FMath::RandRange(0, 999999);
}

void ABed::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABed, UniqueID);
}

void ABed::Interact(AActor* Interactor)
{
	APawn* InteractingPawn = Cast<APawn>(Interactor);
	if (InteractingPawn == nullptr)
	{
		return;
	}

	IBedController* BedController = Cast<IBedController>(InteractingPawn->GetController());
	if (BedController == nullptr)
	{
		return;
	}

	BedController->SetCurrentBed(this->UniqueID, this->GetChunkLocation());

	ATimeOfDayManager* TimeOfDayManager = ATimeOfDayManager::GetTimeOfDayManager();
	if (TimeOfDayManager == nullptr)
	{
		return;
	}

	// if its night, sleep
	if (TimeOfDayManager->IsNight())
	{
		BedController->Sleep();
	}
}

FString ABed::PromptText()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return TEXT("ERROR");
	}

	IBedController* BedController = Cast<IBedController>(World->GetFirstPlayerController());
	if (BedController == nullptr)
	{
		return TEXT("ERROR");
	}

	ATimeOfDayManager* TimeOfDayManager = ATimeOfDayManager::GetTimeOfDayManager();
	if (TimeOfDayManager == nullptr)
	{
		return TEXT("ERROR");
	}

	if (TimeOfDayManager->IsNight())
	{
		// Sleep prompt
		return TEXT("sleep through the night");
	}
	else
	{
		// Spawn prompt
		const bool IsCurrentSpawnPoint = BedController->GetBedUniqueID() == this->UniqueID;
		return IsCurrentSpawnPoint ? TEXT("NOPRESSPROMPT_Current Spawn Point") : TEXT("to set spawn");
	}

}

USceneComponent* ABed::GetSpawnPointComponent() const
{
	return SpawnPointComponent;
}

int32 ABed::GetUniqueID() const
{
	return UniqueID;
}
