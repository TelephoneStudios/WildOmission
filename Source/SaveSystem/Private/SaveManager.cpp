// Copyright Telephone Studios. All Rights Reserved.


#include "SaveManager.h"
#include "Components/PlayerSaveManagerComponent.h"
#include "ChunkManager.h"
#include "TimeOfDayManager.h"
#include "WeatherManager.h"
#include "Interfaces/GameSaveLoadController.h"
#include "WildOmissionSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Log.h"

static ASaveManager* Instance = nullptr;

// Sets default values
ASaveManager::ASaveManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GameSaveLoadController = nullptr;
	CurrentSaveFile = nullptr;

	PlayerSaveManagerComponent = CreateDefaultSubobject<UPlayerSaveManagerComponent>(TEXT("PlayerSaveManagerComponent"));
}

void ASaveManager::SetGameSaveLoadController(IGameSaveLoadController* InGameSaveLoadController)
{
	GameSaveLoadController = InGameSaveLoadController;
}

void ASaveManager::SetSaveManager(ASaveManager* NewInstance)
{
	Instance = NewInstance;
}

ASaveManager* ASaveManager::GetSaveManager()
{
	return Instance;
}

void ASaveManager::SaveWorld()
{
	UWildOmissionSaveGame* SaveFile = GetSaveFile();
	if (SaveFile == nullptr)
	{
		UE_LOG(LogSaveSystem, Error, TEXT("Aborting save, SaveFile was nullptr."));
		return;
	}

	SaveFile->LastPlayedTime = FDateTime::Now();

	AChunkManager* ChunkManager = AChunkManager::GetChunkManager();
	if (ChunkManager)
	{
		SaveFile->PlayerSpawnChunk = ChunkManager->GetPlayerSpawnChunk();
		SaveFile->Seed = ChunkManager->GetGenerationSeed();
		ChunkManager->SaveAllSpawnedChunks();
		SaveFile->ChunkData = ChunkManager->GetAllChunkData();
	}

	ATimeOfDayManager* TimeOfDayManager = ATimeOfDayManager::GetTimeOfDayManager();
	if (TimeOfDayManager)
	{
		SaveFile->DaysPlayed = TimeOfDayManager->GetDaysPlayed();
		SaveFile->NormalizedTimeOfDay = TimeOfDayManager->GetTimeOfDay();
	}

	AWeatherManager* WeatherManager = AWeatherManager::GetWeatherManager();
	if (WeatherManager)
	{
		WeatherManager->Save(SaveFile->WeatherData);
	}

	if (PlayerSaveManagerComponent)
	{
		PlayerSaveManagerComponent->Save(SaveFile->PlayerData);
	}
	
	SaveFile->Version = UWildOmissionSaveGame::GetCurrentVersion();

	UpdateSaveFile(SaveFile);
}

void ASaveManager::SetSaveFile(const FString& SaveFileName)
{
	CurrentSaveFileName = SaveFileName;
	ValidateSave();

	CurrentSaveFile = Cast<UWildOmissionSaveGame>(UGameplayStatics::CreateSaveGameObject(UWildOmissionSaveGame::StaticClass()));
	CurrentSaveFile = Cast<UWildOmissionSaveGame>(UGameplayStatics::LoadGameFromSlot(CurrentSaveFileName + TEXT("/WorldData"), 0));
}

void ASaveManager::LoadWorld()
{
	UWildOmissionSaveGame* SaveFile = GetSaveFile();
	if (SaveFile == nullptr)
	{
		return;
	}
	
	if (SaveFile->CreationInformation.LevelHasGenerated == false)
	{
		SetLoadingSubtitle(TEXT("Generating level."));

		AChunkManager::SetGenerationSeed(SaveFile->Seed);

		SaveFile->CreationInformation.LevelHasGenerated = true;
		UpdateSaveFile(SaveFile);
		return;
	}

	SetLoadingSubtitle(TEXT("Loading level."));

	if (SaveFile->GameMode == 1)
	{
		SaveFile->CheatsEnabled = true;
	}

	UE_LOG(LogTemp, Warning, TEXT("GameMode is %i"), SaveFile->GameMode);

	ATimeOfDayManager* TimeOfDayManager = ATimeOfDayManager::GetTimeOfDayManager();
	if (TimeOfDayManager)
	{
		TimeOfDayManager->SetDaysPlayed(SaveFile->DaysPlayed);
		TimeOfDayManager->SetTimeOfDay(SaveFile->NormalizedTimeOfDay);
	}
	
	AChunkManager* ChunkManager = AChunkManager::GetChunkManager();
	if (ChunkManager)
	{
		ChunkManager->SetPlayerSpawnChunk(SaveFile->PlayerSpawnChunk);
		ChunkManager->SetGenerationSeed(SaveFile->Seed);
		ChunkManager->SetChunkData(SaveFile->ChunkData);
	}

	AWeatherManager* WeatherManager = AWeatherManager::GetWeatherManager();
	if (WeatherManager)
	{
		WeatherManager->Load(SaveFile->WeatherData);
	}
}

UWildOmissionSaveGame* ASaveManager::GetSaveFile() const
{
	return CurrentSaveFile;
}

void ASaveManager::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (World == nullptr || World->IsEditorWorld() && IsValid(Instance))
	{
		return;
	}

	SetSaveManager(this);

	FTimerHandle AutoSaveTimerHandle;
	World->GetTimerManager().SetTimer(AutoSaveTimerHandle, this, &ASaveManager::SaveWorld, 90.0f, true);
}

void ASaveManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	Instance = nullptr;
}

UPlayerSaveManagerComponent* ASaveManager::GetPlayerManager() const
{
	return PlayerSaveManagerComponent;
}

void ASaveManager::ValidateSave()
{
	if (CurrentSaveFileName.Len() > 0)
	{
		UE_LOG(LogSaveSystem, Display, TEXT("Loading into valid save file: %s."), *CurrentSaveFileName);
		return;
	}

	CurrentSaveFileName = TEXT("New_World");
	UE_LOG(LogSaveSystem, Display, TEXT("World Name was 0 in length, using default world name of New_World"));

	if (DoesWorldAlreadExist(CurrentSaveFileName))
	{
		return;
	}

	CreateWorld(CurrentSaveFileName);
}

void ASaveManager::UpdateSaveFile(UWildOmissionSaveGame* UpdatedSaveFile)
{
	if (UpdatedSaveFile == nullptr)
	{
		UE_LOG(LogSaveSystem, Error, TEXT("Aborting update to save file, updated save file passed in was a nullptr."));
		return;
	}

	UGameplayStatics::SaveGameToSlot(UpdatedSaveFile, CurrentSaveFileName + TEXT("/WorldData"), 0);
}

void ASaveManager::StartLoading()
{
	if (GameSaveLoadController == nullptr)
	{
		return;
	}

	GameSaveLoadController->StartLoading();
}

void ASaveManager::StopLoading()
{
	if (GameSaveLoadController == nullptr)
	{
		return;
	}

	GameSaveLoadController->StopLoading();
}

void ASaveManager::SetLoadingTitle(const FString& NewTitle)
{
	if (GameSaveLoadController == nullptr)
	{
		return;
	}

	GameSaveLoadController->SetLoadingTitle(NewTitle);
}

void ASaveManager::SetLoadingSubtitle(const FString& NewSubtitle)
{
	if (GameSaveLoadController == nullptr)
	{
		return;
	}

	GameSaveLoadController->SetLoadingSubtitle(NewSubtitle);
}

void ASaveManager::CreateWorld(const FString& NewWorldName)
{
	if (GameSaveLoadController == nullptr)
	{
		return;
	}

	GameSaveLoadController->CreateWorld(NewWorldName);
}

bool ASaveManager::DoesWorldAlreadExist(const FString& WorldName) const
{
	if (GameSaveLoadController == nullptr)
	{
		return false;
	}

	return GameSaveLoadController->DoesWorldAlreadyExist(WorldName);
}
