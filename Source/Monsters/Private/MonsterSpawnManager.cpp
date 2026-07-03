// Copyright Telephone Studios. All Rights Reserved.


#include "MonsterSpawnManager.h"
#include "SaveManager.h"
#include "TimeOfDayManager.h"
#include "WorldInformation.h"
#include "Engine/DataTable.h"
#include "Monsters/Monster.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

static AMonsterSpawnManager* Instance = nullptr;

// Sets default values
AMonsterSpawnManager::AMonsterSpawnManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MinSpawnCheckTimeSeconds = 5.0f;
	MaxSpawnCheckTimeSeconds = 15.0f;
	InnerSpawnRadiusCentimeters = 5000.0f;
	OuterSpawnRadiusCentimeters = 10000.0f;
	SpawnChance = 1.0f;

	SpawnsAll = true;

	static ConstructorHelpers::FObjectFinder<UDataTable> MonsterSpawnDataTableObject(TEXT("/Game/Monsters/DataTables/DT_MonsterSpawnData"));
	if (MonsterSpawnDataTableObject.Succeeded())
	{
		AISpawnDataTable = MonsterSpawnDataTableObject.Object;
	}
}

AMonsterSpawnManager* AMonsterSpawnManager::GetMonsterSpawnManager()
{
	return Instance;
}

// Called when the game starts or when spawned
void AMonsterSpawnManager::BeginPlay()
{
	Super::BeginPlay();

	const UWorld* World = GetWorld();
	if (World == nullptr || World->IsEditorWorld() && IsValid(Instance))
	{
		return;
	}

	Instance = this;
}

void AMonsterSpawnManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	Instance = nullptr;
}

bool AMonsterSpawnManager::IsSpawnConditionValid()
{
	const ASaveManager* SaveManager = ASaveManager::GetSaveManager();
	if (!IsValid(SaveManager))
	{
		return false;
	}

	const UWorldInformation* SaveFile = SaveManager->GetWorldInformation();
	if (SaveFile == nullptr)
	{
		return false;
	}

	if (SaveFile->Difficulty == EGameDifficulty::EGD_Peaceful)
	{
		return false;
	}

	const ATimeOfDayManager* TimeOfDayManager = ATimeOfDayManager::GetTimeOfDayManager();
	if (!IsValid(TimeOfDayManager))
	{
		return false;
	}

	if (TimeOfDayManager->IsDay())
	{
		return false;
	}

	return true;
}
