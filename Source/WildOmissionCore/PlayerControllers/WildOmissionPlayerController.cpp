// Copyright Telephone Studios. All Rights Reserved.

// Player Controller Core
#include "WildOmissionPlayerController.h"
#include "WildOmissionCore/Characters/WildOmissionCharacter.h"
#include "WildOmissionCore/Characters/WildOmissionSpectatorPawn.h"
#include "GameFramework/PlayerState.h"

// Wild Omission Components
#include "Components/VitalsComponent.h"
#include "Components/InventoryManipulatorComponent.h"
#include "Components/PlayerInventoryComponent.h"
#include "Components/PlayerSaveManagerComponent.h"
#include "Components/MusicPlayerComponent.h"
#include "WildOmissionCore/Components/WOInGameAchievementsComponent.h"

// Wild Omission Stuff
#include "WildOmissionCore/WildOmissionGameInstance.h"
#include "WildOmissionCore/GameModes/WildOmissionGameMode.h"
#include "SaveManager.h"
#include "ChunkManager.h"
#include "Actors/Chunk.h"
#include "Actors/ChunkInvokerActor.h"
#include "GameChatManager.h"
#include "UI/GameplayMenuWidget.h"
#include "WildOmissionCore/UI/Player/DeathMenuWidget.h"

// Engine Stuff
#include "GameFramework/GameSession.h"
#include "Net/UnrealNetwork.h"

AWildOmissionPlayerController::AWildOmissionPlayerController()
{
	bIsStillLoading = true;
	SpawnChunk = FIntVector2();

	TempChunkInvoker = nullptr;

	GameModeIndex = 0;

	BedUniqueID = -1;
	BedChunkLocation = FIntVector2();

	Administrator = false;

	MusicPlayerComponent = nullptr;

	AchievementsComponent = CreateDefaultSubobject<UWOInGameAchievementsComponent>(TEXT("AchievementsComponent"));

	static ConstructorHelpers::FClassFinder<UDeathMenuWidget> DeathMenuWidgetBlueprint(TEXT("/Game/WildOmissionCore/UI/Player/WBP_DeathMenu"));
	if (DeathMenuWidgetBlueprint.Succeeded())
	{
		DeathMenuWidgetClass = DeathMenuWidgetBlueprint.Class;
	}
}

void AWildOmissionPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AWildOmissionPlayerController, GameModeIndex, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AWildOmissionPlayerController, InCheatedWorld, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AWildOmissionPlayerController, BedUniqueID, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AWildOmissionPlayerController, SpawnChunk, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AWildOmissionPlayerController, LastDeathLocation, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AWildOmissionPlayerController, SavedLocation, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AWildOmissionPlayerController, Administrator, COND_OwnerOnly);
}

FPlayerSaveData AWildOmissionPlayerController::SavePlayer()
{
	FPlayerSaveData PlayerSaveData;

	if (!HasAuthority())
	{
		return PlayerSaveData;
	}

	APlayerState* CurrentPlayerState = PlayerState.Get();
	if (CurrentPlayerState == nullptr || AchievementsComponent == nullptr)
	{
		return PlayerSaveData;
	}

	PlayerSaveData.UniqueID = CurrentPlayerState->GetUniqueId().ToString();
	PlayerSaveData.GameMode = GameModeIndex;
	PlayerSaveData.BedUniqueID = BedUniqueID;
	PlayerSaveData.BedChunkLocation = BedChunkLocation;
	PlayerSaveData.AchievementStatsData = AchievementsComponent->GetStatsData();
	PlayerSaveData.NewPlayer = false;

	AWildOmissionCharacter* WildOmissionCharacter = Cast<AWildOmissionCharacter>(GetCharacter());
	if (WildOmissionCharacter == nullptr)
	{
		PlayerSaveData.IsAlive = false;
		return PlayerSaveData;
	}

	PlayerSaveData.WorldLocation = WildOmissionCharacter->GetActorLocation();

	PlayerSaveData.LastDeathLocation = LastDeathLocation;
	PlayerSaveData.SavedLocation = SavedLocation;

	PlayerSaveData.IsAlive = true;
	PlayerSaveData.IsHost = IsHost();

	UVitalsComponent* PlayerVitalsComponent = WildOmissionCharacter->GetVitalsComponent();
	if (PlayerVitalsComponent)
	{
		PlayerSaveData.Vitals.Health = PlayerVitalsComponent->GetHealth();
		PlayerSaveData.Vitals.Hunger = PlayerVitalsComponent->GetHunger();
		PlayerSaveData.Vitals.Thirst = PlayerVitalsComponent->GetThirst();
	}

	UPlayerInventoryComponent* PlayerInventoryComponent = WildOmissionCharacter->GetInventoryComponent();
	if (PlayerInventoryComponent)
	{
		PlayerSaveData.Inventory.ByteData = PlayerInventoryComponent->Save();
	}

	UInventoryManipulatorComponent* PlayerInventoryManipulatorComponent = WildOmissionCharacter->GetInventoryManipulatorComponent();
	if (PlayerInventoryManipulatorComponent)
	{
		PlayerSaveData.SelectedItemByteData = PlayerInventoryManipulatorComponent->GetSelectedItemAsByteData();
	}

	return PlayerSaveData;
}

void AWildOmissionPlayerController::LoadPlayerSave(const FPlayerSaveData& SaveData)
{
	BedUniqueID = SaveData.BedUniqueID;
	BedChunkLocation = SaveData.BedChunkLocation;
	AchievementsComponent->SetStatsData(SaveData.AchievementStatsData);

	LastDeathLocation = SaveData.LastDeathLocation;
	SavedLocation = SaveData.SavedLocation;

	StoredPlayerSaveData = SaveData;
}

bool AWildOmissionPlayerController::IsStillLoading() const
{
	return bIsStillLoading;
}

FString AWildOmissionPlayerController::GetUniqueID() const
{
	FString ID = TEXT("");

	if (PlayerState)
	{
		ID = PlayerState->GetUniqueId().ToString();
	}

	return ID;
}

bool AWildOmissionPlayerController::IsHost() const
{
	return GetLocalRole() == ENetRole::ROLE_Authority && GetRemoteRole() == ENetRole::ROLE_SimulatedProxy;
}

void AWildOmissionPlayerController::SendMessage(APlayerState* Sender, const FString& Message)
{
	Server_SendMessage(Sender, Message);
}

void AWildOmissionPlayerController::Sleep()
{
	// TODO sleep logic here
	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	AWildOmissionGameMode* GameMode = Cast<AWildOmissionGameMode>(World->GetAuthGameMode());
	if (GameMode == nullptr)
	{
		return;
	}

	GameMode->OnPlayerSleep(this);
}

void AWildOmissionPlayerController::SetCurrentBed(const int32& InBedUniqueID, const FIntVector2& InBedChunkLocation)
{
	BedUniqueID = InBedUniqueID;
	BedChunkLocation = InBedChunkLocation;
}

int32 AWildOmissionPlayerController::GetBedUniqueID() const
{
	return BedUniqueID;
}

FIntVector2 AWildOmissionPlayerController::GetBedChunkLocation() const
{
	return BedChunkLocation;
}

void AWildOmissionPlayerController::SetAdministrator(bool InAdministrator)
{
	Administrator = InAdministrator;
}

bool AWildOmissionPlayerController::IsAdministrator() const
{
	return Administrator;
}

void AWildOmissionPlayerController::KickPlayer(APlayerController* PlayerControllerToKick)
{
	Server_KickPlayer(PlayerControllerToKick);
}

void AWildOmissionPlayerController::SetGameModeIndex(const uint8& NewGameModeIndex)
{
	GameModeIndex = NewGameModeIndex;
}

uint8 AWildOmissionPlayerController::GetGameModeIndex() const
{
	return GameModeIndex;
}

void AWildOmissionPlayerController::SetInCheatedWorld(bool CheatedWorld)
{
	InCheatedWorld = CheatedWorld;

	if (AchievementsComponent)
	{
		AchievementsComponent->SetAchievementsEnabled(InCheatedWorld == false);
	}
}

bool AWildOmissionPlayerController::IsInCheatedWorld() const
{
	return InCheatedWorld;
}

bool AWildOmissionPlayerController::IsSurvivalMode() const
{
	return GameModeIndex == 0;
}

bool AWildOmissionPlayerController::IsCreativeMode() const
{
	return GameModeIndex == 1;
}

void AWildOmissionPlayerController::Save()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	Server_AddToPendingSaves();

	if (!HasAuthority())
	{
		return;
	}
	
	AWildOmissionGameMode* GameMode = Cast<AWildOmissionGameMode>(World->GetAuthGameMode());
	if (GameMode == nullptr)
	{
		return;
	}

	GameMode->SaveWorld();
}

bool AWildOmissionPlayerController::IsEditorPlayer() const
{
#if WITH_EDITOR
	return true;
#else
	return false;
#endif
}

void AWildOmissionPlayerController::Server_AddToPendingSaves_Implementation()
{
	ASaveManager* SaveManager = ASaveManager::GetSaveManager();
	if (SaveManager == nullptr)
	{
		return;
	}

	UPlayerSaveManagerComponent* PlayerSaveManager = SaveManager->GetPlayerManager();
	if (PlayerSaveManager == nullptr)
	{
		return;
	}

	PlayerSaveManager->AddToPending(this);
}

void AWildOmissionPlayerController::Server_KillThisPlayer_Implementation()
{
	// Get pawn
	APawn* OurPawn = GetPawn();
	if (OurPawn == nullptr)
	{
		return;
	}

	// Get vitals from pawn
	UVitalsComponent* PawnVitals = OurPawn->FindComponentByClass<UVitalsComponent>();
	if (PawnVitals == nullptr)
	{
		return;
	}

	// Set health to zero
	PawnVitals->SetHealth(0.0f);
}

//*****************************
// Console functions

void AWildOmissionPlayerController::OnPlayerDeath(const FVector& DeathLocation)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	SetLastDeathLocation(DeathLocation);

	TempChunkInvoker = World->SpawnActor<AChunkInvokerActor>(AChunkInvokerActor::StaticClass(), DeathLocation, FRotator::ZeroRotator);
}

void AWildOmissionPlayerController::Kill()
{
	Server_KillThisPlayer();
}

void AWildOmissionPlayerController::LogLocalInventoryContents()
{
	if (!IsLocalController())
	{
		return;
	}
	AWildOmissionCharacter* WildOmissionCharacter = Cast<AWildOmissionCharacter>(GetPawn());
	if (WildOmissionCharacter == nullptr)
	{
		return;
	}

	// Get their inventory components
	for (const FInventoryItem& ItemData : GetPawn()->FindComponentByClass<UPlayerInventoryComponent>()->GetContents()->Contents)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.0f, FColor::Orange, FString::Printf(TEXT("Item: %s, Quantity: %i"), *ItemData.Name.ToString(), ItemData.Quantity));
	}
	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.0f, FColor::Green, FString::Printf(TEXT("Player: %s"), *WildOmissionCharacter->GetActorNameOrLabel()));
}

void AWildOmissionPlayerController::SetSavedLocationToCurrentLocation()
{
	Server_SetSavedLocationToCurrentLocation();
}

void AWildOmissionPlayerController::SetLastDeathLocation(const FVector& DeathLocation)
{
	LastDeathLocation = DeathLocation / AChunk::GetVertexDistanceScale();
}

FVector AWildOmissionPlayerController::GetCurrentCoordinateLocation()
{
	APawn* OurPawn = GetPawn();
	if (!IsValid(OurPawn))
	{
		return FVector::ZeroVector;
	}

	return OurPawn->GetActorLocation() / AChunk::GetVertexDistanceScale();
}

FVector AWildOmissionPlayerController::GetSavedLocation()
{
	return SavedLocation;
}

FVector AWildOmissionPlayerController::GetLastDeathLocation()
{
	return LastDeathLocation;
}

void AWildOmissionPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SetupPlayerOnServer();
	}

	if (!IsLocalController())
	{
		return;
	}
	
	StartLoading();

	SetupMusicPlayerComponent();

	HasAuthority() ? SetupLocalAsHost() : SetupLocalAsClient();
}

void AWildOmissionPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	if (AWildOmissionSpectatorPawn* WildOmissionSpectatorPawn = Cast<AWildOmissionSpectatorPawn>(aPawn))
	{
		TempChunkInvoker = 
			World->SpawnActor<AChunkInvokerActor>
			(AChunkInvokerActor::StaticClass(),
			WildOmissionSpectatorPawn->GetActorLocation(),
			WildOmissionSpectatorPawn->GetActorRotation());
		return;
	}

	AWildOmissionCharacter* WildOmissionCharacter = Cast<AWildOmissionCharacter>(aPawn);
	if (WildOmissionCharacter == nullptr)
	{
		return;
	}

	WildOmissionCharacter->OnPlayerDeath.AddDynamic(this, &AWildOmissionPlayerController::OnPlayerDeath);

	if (bIsStillLoading == false || StoredPlayerSaveData.IsAlive == false || StoredPlayerSaveData.NewPlayer == true)
	{
		return;
	}

	WildOmissionCharacter->SetActorLocation(StoredPlayerSaveData.WorldLocation);

	UVitalsComponent* CharacterVitalsComponent = WildOmissionCharacter->GetVitalsComponent();
	if (CharacterVitalsComponent == nullptr)
	{
		return;
	}

	CharacterVitalsComponent->SetHealth(StoredPlayerSaveData.Vitals.Health);
	CharacterVitalsComponent->SetHunger(StoredPlayerSaveData.Vitals.Hunger);
	CharacterVitalsComponent->SetThirst(StoredPlayerSaveData.Vitals.Thirst);

	UInventoryComponent* CharacterInventoryComponent = WildOmissionCharacter->GetInventoryComponent();
	UInventoryManipulatorComponent* CharacterInventoryManipulatorComponent = WildOmissionCharacter->GetInventoryManipulatorComponent();
	if (CharacterInventoryComponent == nullptr || CharacterInventoryManipulatorComponent == nullptr)
	{
		return;
	}

	CharacterInventoryComponent->Load(StoredPlayerSaveData.Inventory.ByteData);
	CharacterInventoryManipulatorComponent->LoadSelectedItemFromByteDataAndDropInWorld(StoredPlayerSaveData.SelectedItemByteData);

	StoredPlayerSaveData = FPlayerSaveData();

	if (TempChunkInvoker == nullptr)
	{
		return;
	}

	TempChunkInvoker->Destroy();
	TempChunkInvoker = nullptr;
}

void AWildOmissionPlayerController::SetupPlayerOnServer()
{
	UWorld* World = GetWorld();
	AChunkManager* ChunkManager = AChunkManager::GetChunkManager();
	if (World == nullptr || ChunkManager == nullptr)
	{
		return;
	}

	const bool UseDefaultSpawn = StoredPlayerSaveData.IsAlive == false || StoredPlayerSaveData.NewPlayer == true;
	const FVector SpawnPoint = UseDefaultSpawn ? ChunkManager->GetWorldSpawnPoint() : StoredPlayerSaveData.WorldLocation;

	// Spawn Wild Omission spectator (Has chunk invoker)
	AWildOmissionSpectatorPawn* SpecPawn = World->SpawnActor<AWildOmissionSpectatorPawn>(AWildOmissionSpectatorPawn::StaticClass(), SpawnPoint, FRotator::ZeroRotator);
	if (SpecPawn)
	{
		this->Possess(SpecPawn);
	}

	const float ChunkSize = AChunk::GetVertexSize() * AChunk::GetVertexDistanceScale();
	SpawnChunk = FIntVector2(
		FMath::RoundToInt32(SpawnPoint.X / ChunkSize),
		FMath::RoundToInt32(SpawnPoint.Y / ChunkSize));
}

void AWildOmissionPlayerController::StartLoading()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	UWildOmissionGameInstance* GameInstance = UWildOmissionGameInstance::GetWildOmissionGameInstance(World);
	if (GameInstance == nullptr)
	{
		return;
	}

	GameInstance->StartLoading();
	if (!IsHost())
	{
		GameInstance->SetLoadingSubtitle(TEXT("Loading world state."));
	}
	UE_LOG(LogPlayerController, Verbose, TEXT("BeginPlay: Brought up loading screen."));
}

void AWildOmissionPlayerController::SetupMusicPlayerComponent()
{
	if (MusicPlayerComponent != nullptr)
	{
		return;
	}

	MusicPlayerComponent = NewObject<UMusicPlayerComponent>(this, UMusicPlayerComponent::StaticClass(), TEXT("MusicPlayerComponent"));
	if (MusicPlayerComponent == nullptr)
	{
		return;
	}
	
	MusicPlayerComponent->RegisterComponent();
}

void AWildOmissionPlayerController::SetupLocalAsHost()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	FTimerHandle LoadTimerHandle;
	FTimerDelegate LoadTimerDelegate;
	LoadTimerDelegate.BindUObject(this, &AWildOmissionPlayerController::StopLoading);
	World->GetTimerManager().SetTimer(LoadTimerHandle, LoadTimerDelegate, 2.0f, false);
	UE_LOG(LogPlayerController, Verbose, TEXT("BeginPlay: Setup load timer."));
}

void AWildOmissionPlayerController::SetupLocalAsClient()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	FTimerDelegate CheckSpawnChunkValidTimerDelegate;
	CheckSpawnChunkValidTimerDelegate.BindUObject(this, &AWildOmissionPlayerController::CheckSpawnChunkValid);
	World->GetTimerManager().SetTimer(CheckSpawnChunkValidTimerHandle, CheckSpawnChunkValidTimerDelegate, 2.0f, true);
	UE_LOG(LogPlayerController, Verbose, TEXT("BeginPlay: Setup check spawn chunk timer."));
}

void AWildOmissionPlayerController::CheckSpawnChunkValid()
{
	UWorld* World = GetWorld();
	AChunkManager* ChunkManager = AChunkManager::GetChunkManager();
	if (World == nullptr || ChunkManager == nullptr)
	{
		UE_LOG(LogPlayerController, Warning, TEXT("CheckSpawnChunkValid: Failed because World or ChunkManager is nullptr."))
		return;
	}

	FSpawnedChunk SpawnedChunk;
	ChunkManager->GetSpawnedChunk(SpawnChunk, SpawnedChunk);

	const int32 DataCount = (AChunk::GetVertexSize() + 1) * (AChunk::GetVertexSize() + 1);
	if (!IsValid(SpawnedChunk.Chunk) || SpawnedChunk.Chunk->GetHeightData().Num() != DataCount)
	{
		UE_LOG(LogPlayerController, Verbose, TEXT("CheckSpawnChunkValid: Chunk is still loading."));
		return;
	}

	UE_LOG(LogPlayerController, Verbose, TEXT("CheckSpawnChunkValid: Chunk loaded successfully, requesting spawn."));
	World->GetTimerManager().ClearTimer(CheckSpawnChunkValidTimerHandle);
	StopLoading();
}

void AWildOmissionPlayerController::StopLoading()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	Server_Spawn();
	
	UWildOmissionGameInstance* GameInstance = UWildOmissionGameInstance::GetWildOmissionGameInstance(World);
	if (GameInstance == nullptr)
	{
		UE_LOG(LogPlayerController, Warning, TEXT("Couldnt Stop Loading, GameInstance returned a nullptr."));
		return;
	}

	GameInstance->StopLoading();
}

void AWildOmissionPlayerController::Server_SetSavedLocationToCurrentLocation_Implementation()
{
	APawn* OurPawn = GetPawn();
	if (!IsValid(OurPawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to save current location, player controller doesn't have a pawn."));
		return;
	}

	SavedLocation = OurPawn->GetActorLocation() / AChunk::GetVertexDistanceScale();
}

void AWildOmissionPlayerController::Server_SendMessage_Implementation(APlayerState* Sender, const FString& Message)
{
	AGameChatManager* ChatManager = AGameChatManager::GetGameChatManager();
	if (ChatManager == nullptr)
	{
		return;
	}

	ChatManager->SendMessage(Sender, Message, EChatMessageType::STANDARD);
}

bool AWildOmissionPlayerController::Server_KickPlayer_Validate(APlayerController* PlayerControllerToKick)
{
	if (!this->IsAdministrator())
	{
		return false;
	}

	return true;
}

void AWildOmissionPlayerController::Server_KickPlayer_Implementation(APlayerController* PlayerControllerToKick)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	AGameModeBase* GameMode = World->GetAuthGameMode();
	if (GameMode == nullptr)
	{
		return;
	}

	AGameSession* GameSession = GameMode->GameSession.Get();
	if (GameSession == nullptr)
	{
		return;
	}

	if (PlayerControllerToKick == nullptr)
	{
		UE_LOG(LogPlayerController, Warning, TEXT("Failed to kick player, couldn't get player controller."));
		return;
	}

	GameSession->KickPlayer(PlayerControllerToKick, FText::FromString(TEXT("Kicked by Server Administrator.")));
}

void AWildOmissionPlayerController::Server_Spawn_Implementation()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	AWildOmissionGameMode* GameMode = Cast<AWildOmissionGameMode>(World->GetAuthGameMode());
	if (GameMode && (StoredPlayerSaveData.IsAlive || StoredPlayerSaveData.NewPlayer))
	{
		GameMode->SpawnHumanForController(this);
	}
	else
	{
		Client_ShowDeathMenu();
	}

	if (bIsStillLoading == true)
	{
		bIsStillLoading = false;
	}
}

void AWildOmissionPlayerController::Client_ShowDeathMenu_Implementation()
{
	UWorld* World = GetWorld();
	if (World == nullptr || !IsLocalController())
	{
		return;
	}

	// Make Sure to Close Pause Menu So it Doesn't Get Stuck on Screen!
	UWildOmissionGameInstance* WOGameInstance = UWildOmissionGameInstance::GetWildOmissionGameInstance(World);
	if (WOGameInstance)
	{
		UGameplayMenuWidget* OpenGameplayMenu = WOGameInstance->GetGameplayMenuWidget();
		if (OpenGameplayMenu)
		{
			OpenGameplayMenu->Teardown();
		}
	}
	
	UE_LOG(LogPlayerController, Verbose, TEXT("Bringing Up Death Menu."));

	UDeathMenuWidget* DeathMenu = CreateWidget<UDeathMenuWidget>(this, DeathMenuWidgetClass);
	if (DeathMenu == nullptr)
	{
		UE_LOG(LogPlayerController, Warning, TEXT("Failed to show death menu."));
		return;
	}

	DeathMenu->Show();
}