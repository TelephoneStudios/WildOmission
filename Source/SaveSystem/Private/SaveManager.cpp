// Copyright Telephone Studios. All Rights Reserved.


#include "SaveManager.h"
#include "Components/PlayerSaveManagerComponent.h"
#include "ChunkManager.h"
#include "TimeOfDayManager.h"
#include "WeatherManager.h"
#include "Interfaces/GameSaveLoadController.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "WorldInformation.h"
#include "WorldData.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Log.h"

static ASaveManager* SaveManagerInstance = nullptr;

// Sets default values
ASaveManager::ASaveManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GameSaveLoadController = nullptr;

	CurrentWorldInformation = nullptr;
	CurrentWorldData = nullptr;

	PlayerSaveManagerComponent = CreateDefaultSubobject<UPlayerSaveManagerComponent>(TEXT("PlayerSaveManagerComponent"));
}

void ASaveManager::SetGameSaveLoadController(IGameSaveLoadController* InGameSaveLoadController)
{
	GameSaveLoadController = InGameSaveLoadController;
}

void ASaveManager::SetSaveManager(ASaveManager* NewSaveManagerInstance)
{
	SaveManagerInstance = NewSaveManagerInstance;
}

ASaveManager* ASaveManager::GetSaveManager()
{
	return SaveManagerInstance;
}

void ASaveManager::SaveWorld()
{
	if (CurrentWorldData == nullptr
		|| CurrentWorldInformation == nullptr)
	{
		UE_LOG(LogSaveSystem, Error, TEXT("Aborting save, CurrentWorldInformation or CurrentWorldData was nullptr."));
		return;
	}

	// Populate World Information
	CurrentWorldInformation->LastPlayedTime = FDateTime::Now();
	
	// Save Chunks
	AChunkManager* ChunkManager = AChunkManager::GetChunkManager();
	if (ChunkManager)
	{
		// Populate World Information
		CurrentWorldInformation->Seed = ChunkManager->GetGenerationSeed();
		
		// Populate World Data
		CurrentWorldData->PlayerSpawnChunk = ChunkManager->GetPlayerSpawnChunk();
		
		ChunkManager->SaveAllSpawnedChunks();
		CurrentWorldData->ChunkData = ChunkManager->GetAllChunkData();
	}

	// Save TimeOfDay
	ATimeOfDayManager* TimeOfDayManager = ATimeOfDayManager::GetTimeOfDayManager();
	if (TimeOfDayManager)
	{
		// Populate World Information
		CurrentWorldInformation->DaysPlayed = TimeOfDayManager->GetDaysPlayed();
		CurrentWorldInformation->NormalizedTimeOfDay = TimeOfDayManager->GetTimeOfDay();
	}

	// Save Weather
	AWeatherManager* WeatherManager = AWeatherManager::GetWeatherManager();
	if (WeatherManager)
	{
		WeatherManager->Save(CurrentWorldData->WeatherData);
	}

	// Save Players
	if (PlayerSaveManagerComponent)
	{
		PlayerSaveManagerComponent->Save(CurrentWorldData->PlayerData);
	}
	
	CurrentWorldInformation->Version = UWorldInformation::GetCurrentVersion();

	// Capture World Icon
	CaptureWorldIcon();

	UpdateWorldFile(CurrentWorldInformation, CurrentWorldData);
}

void ASaveManager::SetWorld(const FString& WorldName)
{
	CurrentWorldName = WorldName;
	ValidateSave();

	CurrentWorldInformation = Cast<UWorldInformation>(UGameplayStatics::CreateSaveGameObject(UWorldInformation::StaticClass()));
	CurrentWorldInformation = Cast<UWorldInformation>(UGameplayStatics::LoadGameFromSlot(CurrentWorldName + TEXT("/WorldInformation"), 0));

	CurrentWorldData = Cast<UWorldData>(UGameplayStatics::CreateSaveGameObject(UWorldData::StaticClass()));
	CurrentWorldData = Cast<UWorldData>(UGameplayStatics::LoadGameFromSlot(CurrentWorldName + TEXT("/WorldData"), 0));
}

void ASaveManager::LoadWorld()
{
	if (CurrentWorldInformation == nullptr || CurrentWorldData == nullptr)
	{
		UE_LOG(LogSaveSystem, Warning, TEXT("Attemped to load world but CurrentWorldInformation or CurrentWorldData was nullptr."));
		return;
	}
	
	// Check if the world needs to be generated first
	if (CurrentWorldInformation->CreationInformation.LevelHasGenerated == false)
	{
		// Generate world
		SetLoadingSubtitle(TEXT("Generating level."));

		AChunkManager::SetGenerationSeed(CurrentWorldInformation->Seed);

		CurrentWorldInformation->CreationInformation.LevelHasGenerated = true;
		UpdateWorldFile(CurrentWorldInformation, CurrentWorldData);
		return;
	}

	SetLoadingSubtitle(TEXT("Loading level."));

	if (CurrentWorldInformation->GameMode == 1)
	{
		CurrentWorldInformation->CheatsEnabled = true;
	}

	UE_LOG(LogSaveSystem, Warning, TEXT("Loading world GameMode is %i"), CurrentWorldInformation->GameMode);

	ATimeOfDayManager* TimeOfDayManager = ATimeOfDayManager::GetTimeOfDayManager();
	if (TimeOfDayManager)
	{
		TimeOfDayManager->SetDaysPlayed(CurrentWorldInformation->DaysPlayed);
		TimeOfDayManager->SetTimeOfDay(CurrentWorldInformation->NormalizedTimeOfDay);
	}
	
	AChunkManager* ChunkManager = AChunkManager::GetChunkManager();
	if (ChunkManager)
	{
		ChunkManager->SetPlayerSpawnChunk(CurrentWorldData->PlayerSpawnChunk);
		ChunkManager->SetGenerationSeed(CurrentWorldInformation->Seed);
		ChunkManager->SetChunkData(CurrentWorldData->ChunkData);
	}

	AWeatherManager* WeatherManager = AWeatherManager::GetWeatherManager();
	if (WeatherManager)
	{
		WeatherManager->Load(CurrentWorldData->WeatherData);
	}
}

void ASaveManager::UpdateWorldFile(UWorldInformation* UpdatedWorldInformation, UWorldData* UpdatedWorldData)
{
	UpdateWorldInformation(UpdatedWorldInformation);
	UpdateWorldData(UpdatedWorldData);
}

void ASaveManager::UpdateWorldInformation(UWorldInformation* UpdatedWorldInformation)
{
	if (UpdatedWorldInformation == nullptr)
	{
		UE_LOG(LogSaveSystem, Error, TEXT("Aborting update to world files, UpdatedWorldInformation was nullptr."));
		return;
	}

	UGameplayStatics::SaveGameToSlot(UpdatedWorldInformation, CurrentWorldName + TEXT("/WorldInformation"), 0);
}

void ASaveManager::UpdateWorldData(UWorldData* UpdatedWorldData)
{
	if (UpdatedWorldData == nullptr)
	{
		UE_LOG(LogSaveSystem, Error, TEXT("Aborting update to world files, UpdatedWorldData was nullptr."));
		return;
	}

	UGameplayStatics::SaveGameToSlot(UpdatedWorldData, CurrentWorldName + TEXT("/WorldData"), 0);
}

void ASaveManager::CaptureWorldIcon()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		UE_LOG(LogSaveSystem, Error, TEXT("Failed to capture world thumbnail, world was nullptr"));
		return;
	}

	// Create 512 x 512 render target
	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>(World);
	RenderTarget->InitAutoFormat(512, 512);
	RenderTarget->TargetGamma = 2.2f;
	RenderTarget->UpdateResource();

	// Spawn a temporary Scene Capture Component from the player's camera view
	APlayerController* FirstPlayerController = World->GetFirstPlayerController();
	if (FirstPlayerController == nullptr || FirstPlayerController->PlayerCameraManager == nullptr)
	{
		UE_LOG(LogSaveSystem, Warning, TEXT("Failed to capture world thumbnail, FirstPlayerController was nullptr"));
		return;
	}

	USceneCaptureComponent2D* CaptureComponent = NewObject<USceneCaptureComponent2D>(FirstPlayerController);
	CaptureComponent->RegisterComponent();

	CaptureComponent->SetWorldLocationAndRotation(
		FirstPlayerController->PlayerCameraManager->GetCameraLocation(),
		FirstPlayerController->PlayerCameraManager->GetCameraRotation()
	);

	// Configure capture settings
	CaptureComponent->TextureTarget = RenderTarget;
	CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	CaptureComponent->bCaptureEveryFrame = false;
	CaptureComponent->bCaptureOnMovement = false;
	CaptureComponent->bAlwaysPersistRenderingState = true;

	// Override individual ShowFlags to ensure GI and Lumen are allowed in the render loop
	FEngineShowFlags& ShowFlags = CaptureComponent->ShowFlags;
	ShowFlags.SetGlobalIllumination(true);
	ShowFlags.SetLumenGlobalIllumination(true);
	ShowFlags.SetLumenReflections(true);
	ShowFlags.SetAmbientOcclusion(true);
	ShowFlags.SetDirectLighting(true);
	ShowFlags.SetIndirectLightingCache(true);

	// Capture
	CaptureComponent->CaptureScene();

	// Read the raw pixel data from the render target
	TArray<FColor> RawPixels;
	FTextureRenderTargetResource* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
	RenderTargetResource->ReadPixels(RawPixels);

	// Compress raw pixel array into PNG format
	TArray64<uint8> CompressedPNG;
	FImageUtils::PNGCompressImageArray(512, 512, RawPixels, CompressedPNG);

	// Save the PNG into the world directory
	const FString FilePath = FPaths::ProjectSavedDir() + "/SaveGames/" + CurrentWorldName + "/Icon.png";
	FFileHelper::SaveArrayToFile(CompressedPNG, *FilePath);

	// Clean up
	CaptureComponent->DestroyComponent();
}

UTexture2D* ASaveManager::GetWorldIcon(const FString& WorldName)
{
	// Build file path
	const FString FilePath = FPaths::ProjectSavedDir() + "/SaveGames/" + WorldName + "/Icon.png";
	// Check if it actually exists
	if (!FPaths::FileExists(FilePath))
	{
		UE_LOG(LogSaveSystem, Warning, TEXT("World icon not found at: %s"), *FilePath);
		return nullptr;
	}

	// Load the raw compressed binary data from disk
	TArray<uint8> RawFileData;
	if (!FFileHelper::LoadFileToArray(RawFileData, *FilePath))
	{
		return nullptr;
	}

	// Convert png back into texture
	UTexture2D* LoadedTexture = FImageUtils::ImportBufferAsTexture2D(RawFileData);

	return LoadedTexture;
}

UWorldInformation* ASaveManager::GetWorldInformation() const
{
	return CurrentWorldInformation;
}

UWorldData* ASaveManager::GetWorldData() const
{
	return CurrentWorldData;
}


TArray<FString> ASaveManager::GetAllWorldNamesV1()
{
	////////////////////////////////////////////////////////////////////////////////////
	// Special thanks to Ixiguis on the Unreal Engine forums for this useful function //
	////////////////////////////////////////////////////////////////////////////////////
	class FFindSavesVisitor : public IPlatformFile::FDirectoryVisitor
	{
	public:
		FFindSavesVisitor() {}

		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory)
		{
			if (!bIsDirectory)
			{
				FString FullFilePath(FilenameOrDirectory);
				if (FPaths::GetExtension(FullFilePath) == TEXT("sav"))
				{
					FString CleanFilename = FPaths::GetBaseFilename(FullFilePath);
					CleanFilename = CleanFilename.Replace(TEXT(".sav"), TEXT(""));
					SavesFound.Add(CleanFilename);
				}
			}
			return true;
		}
		TArray<FString> SavesFound;
	};

	TArray<FString> Saves;
	const FString SavesFolder = FPaths::ProjectSavedDir() + TEXT("SaveGames");

	if (!SavesFolder.IsEmpty())
	{
		FFindSavesVisitor Visitor;
		FPlatformFileManager::Get().GetPlatformFile().IterateDirectory(*SavesFolder, Visitor);
		Saves = Visitor.SavesFound;
	}

	return Saves;
}

TArray<FString> ASaveManager::GetAllWorldFolderNames()
{
	TArray<FString> FolderNames;
	FString SearchDirectory = FPaths::ProjectSavedDir() + TEXT("SaveGames/") + TEXT("*");

	IFileManager::Get().FindFiles(FolderNames, *SearchDirectory, false, true);

	return FolderNames;
}

bool ASaveManager::WorldAlreadyExists(const FString& WorldNameToTest)
{
	TArray<FString> WorldNames = GetAllWorldFolderNames();
	for (const FString& WorldName : WorldNames)
	{
		if (WorldNameToTest.ToLower() == WorldName.ToLower())
		{
			return true;
		}
	}

	return false;
}


void ASaveManager::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (World == nullptr || World->IsEditorWorld() && IsValid(SaveManagerInstance))
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

	SaveManagerInstance = nullptr;
}

UPlayerSaveManagerComponent* ASaveManager::GetPlayerManager() const
{
	return PlayerSaveManagerComponent;
}

void ASaveManager::ValidateSave()
{
	if (CurrentWorldName.Len() > 0)
	{
		UE_LOG(LogSaveSystem, Display, TEXT("Loading into valid world: %s."), *CurrentWorldName);
		return;
	}

	// If loaded world name is blank create populate it with "New_World"
	CurrentWorldName = TEXT("New_World");
	UE_LOG(LogSaveSystem, Display, TEXT("World Name was 0 in length, using default world name of New_World"));

	if (DoesWorldAlreadExist(CurrentWorldName))
	{
		return;
	}

	CreateWorld(CurrentWorldName);
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
