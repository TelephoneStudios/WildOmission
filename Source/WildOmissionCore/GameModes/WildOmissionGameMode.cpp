// Copyright Telephone Studios. All Rights Reserved.


#include "WildOmissionGameMode.h"
#include "SaveManager.h"
#include "Components/PlayerSaveManagerComponent.h"
#include "ServerAdministrators.h"
#include "Deployables/Bed.h"
#include "ChunkManager.h"
#include "Actors/Chunk.h"
#include "TimeOfDayManager.h"
#include "WeatherManager.h"
#include "AnimalSpawnManager.h"
#include "MonsterSpawnManager.h"
#include "GameChatManager.h"
#include "WildOmissionCore/WildOmissionGameInstance.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineFriendsInterface.h" 
#include "OnlineSessionSettings.h"
#include "WildOmissionCore/WildOmissionGameState.h"
#include "WildOmissionCore/PlayerControllers/WildOmissionPlayerController.h"
#include "WildOmissionCore/Characters/WildOmissionCharacter.h"
#include "WildOmissionCore/Components/WOInGameAchievementsComponent.h"
#include "AchievementsManager.h"
#include "Components/PlayerInventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerState.h"
#include "Actors/Storm.h"

void AWildOmissionGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	FString SaveFile = UGameplayStatics::ParseOption(Options, "SaveGame");
	FriendsOnly = UGameplayStatics::ParseOption(Options, "FriendsOnly") == TEXT("1");
	
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	SaveManager = World->SpawnActor<ASaveManager>();
	ASaveManager::SetSaveManager(SaveManager);

	ChunkManager = World->SpawnActor<AChunkManager>();
	AChunkManager::SetChunkManager(ChunkManager);

	TimeOfDayManager = World->SpawnActor<ATimeOfDayManager>();
	ATimeOfDayManager::SetTimeOfDayManager(TimeOfDayManager);

	WeatherManager = World->SpawnActor<AWeatherManager>();
	AWeatherManager::SetWeatherManager(WeatherManager);

	AnimalSpawnManager = World->SpawnActor<AAnimalSpawnManager>();
	MonsterSpawnManager = World->SpawnActor<AMonsterSpawnManager>();
	
	ChatManager = World->SpawnActor<AGameChatManager>();
	AGameChatManager::SetGameChatManager(ChatManager);

	if (SaveManager == nullptr)
	{
		return;
	}

	SaveManager->SetGameSaveLoadController(Cast<IGameSaveLoadController>(GetGameInstance()));
	SaveManager->SetSaveFile(SaveFile);
	SaveManager->LoadWorld();
}

void AWildOmissionGameMode::StartPlay()
{
	Super::StartPlay();

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	UWildOmissionGameInstance* GameInstance = UWildOmissionGameInstance::GetWildOmissionGameInstance(World);
	if (GameInstance == nullptr || !GameInstance->IsDedicatedServerInstance())
	{
		return;
	}

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem == nullptr)
	{
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		return;
	}

	FOnlineSessionSettings SessionSettings;
	SessionSettings.NumPublicConnections = 100;
	SessionSettings.bIsDedicated = true;
	SessionSettings.bIsLANMatch = false;
	SessionSettings.bUsesPresence = false;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.BuildUniqueId = 1;

	const FString ServerName = TEXT("Dedicated Server (WIP)");
	const FString LevelFile = TEXT("LV_Procedural");

	SessionSettings.Set(UWildOmissionGameInstance::GetFriendsOnlySettingsKey(), false, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set(UWildOmissionGameInstance::GetServerNameSettingsKey(), ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set(UWildOmissionGameInstance::GetLevelFileSettingsKey(), LevelFile, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set(TEXT("NAME"), ServerName, EOnlineDataAdvertisementType::Type::ViaOnlineServiceAndPing);
	SessionSettings.Set(TEXT("MAPNAME"), LevelFile, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set(UWildOmissionGameInstance::GetGameVersionSettingsKey(), GameInstance->GetVersion(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	SessionInterface->CreateSession(0, UWildOmissionGameInstance::GetSessionName(), SessionSettings);
}

void AWildOmissionGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	UWildOmissionGameInstance* GameInstance = UWildOmissionGameInstance::GetWildOmissionGameInstance(World);
	IOnlineFriendsPtr FriendsInterface = GameInstance->GetFriendsInterface();
	if (FriendsOnly && GameInstance && FriendsInterface && !FriendsInterface->IsFriend(0, *UniqueId.GetUniqueNetId().Get(), FString()))
	{
		ErrorMessage = TEXT("Player not friend, revoking connection attempt.");
	}

	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
}

void AWildOmissionGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AWildOmissionPlayerController* NewWildOmissionPlayer = Cast<AWildOmissionPlayerController>(NewPlayer);
	if (NewWildOmissionPlayer == nullptr)
	{
		return;
	}

	ProcessMultiplayerJoinAchievement(NewWildOmissionPlayer);

	UPlayerSaveManagerComponent* PlayerSaveManager = SaveManager->GetPlayerManager();
	if (PlayerSaveManager)
	{
		PlayerSaveManager->Load(NewPlayer);
	}


	APlayerState* NewPlayerState = NewPlayer->GetPlayerState<APlayerState>();
	UServerAdministrators* ServerAdministrators = UServerAdministrators::Get();
	if (NewPlayerState == nullptr || ServerAdministrators == nullptr)
	{
		return;
	}

	const bool IsNewPlayerHost = NewWildOmissionPlayer->IsHost();
	const bool IsNewPlayerAdministrator = ServerAdministrators->IsAdministrator(NewPlayerState->GetUniqueId().ToString());
	NewWildOmissionPlayer->SetAdministrator(IsNewPlayerHost || IsNewPlayerAdministrator);

	if (ChatManager == nullptr || (GetWorld() && GetWorld()->IsPlayInEditor()))
	{
		return;
	}

	ChatManager->SendMessage(NewPlayerState, TEXT("Has Joined The Game."), true);
}

void AWildOmissionGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (GetWorld() && GetWorld()->IsPlayInEditor())
	{
		return;
	}

	APlayerState* ExitingPlayerState = Exiting->GetPlayerState<APlayerState>();
	if (ChatManager == nullptr || ExitingPlayerState == nullptr)
	{
		return;
	}

	ChatManager->SendMessage(ExitingPlayerState, TEXT("Has Left The Game."), true);
}

void AWildOmissionGameMode::SpawnHumanForController(APlayerController* Controller)
{
	if (Controller == nullptr || !IsValid(Controller))
	{
		UE_LOG(LogGameMode, Warning, TEXT("Failed to spawn Human, PlayerController wasn't valid."));
		return;
	}

	if (APawn* Pawn = Controller->GetPawnOrSpectator())
	{
		Controller->UnPossess();
		Pawn->Destroy();
	}

	AWildOmissionPlayerController* WOPlayerController = Cast<AWildOmissionPlayerController>(Controller);
	if (WOPlayerController && WOPlayerController->GetBedUniqueID() != -1)
	{
		SpawnHumanAtBed(Controller);
	}
	else
	{
		SpawnHumanAtStartSpot(Controller);
	}
}

void AWildOmissionGameMode::SaveWorld()
{
	SaveManager->SaveWorld();
}

void AWildOmissionGameMode::TeleportAllPlayersToSelf()
{
	APlayerController* SelfPlayerController = GetWorld()->GetFirstPlayerController();
	if (SelfPlayerController == nullptr)
	{
		return;
	}

	APawn* SelfPlayerPawn = SelfPlayerController->GetPawn();
	if (SelfPlayerPawn == nullptr)
	{
		return;
	}
	
	const FVector TargetLocation = SelfPlayerPawn->GetActorLocation();

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController == nullptr)
		{
			continue;
		}

		APawn* PlayerPawn = PlayerController->GetPawn();
		if (PlayerPawn == nullptr || PlayerPawn == SelfPlayerPawn)
		{
			continue;
		}
	
		PlayerPawn->SetActorLocation(TargetLocation);
	}
}

void AWildOmissionGameMode::GiveSelfGodMode()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	APlayerController* SelfPlayerController = World->GetFirstPlayerController();
	if (SelfPlayerController == nullptr)
	{
		return;
	}

	AWildOmissionCharacter* SelfCharacter = Cast<AWildOmissionCharacter>(SelfPlayerController->GetPawn());
	if (SelfCharacter == nullptr)
	{
		return;
	}

	SelfCharacter->SetGodMode(true);
}

void AWildOmissionGameMode::GiveAllPlayersGodMode()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController == nullptr)
		{
			return;
		}

		AWildOmissionCharacter* PlayerCharacter = Cast<AWildOmissionCharacter>(PlayerController->GetPawn());
		if (PlayerCharacter == nullptr)
		{
			return;
		}

		PlayerCharacter->SetGodMode(true);
	}
}

void AWildOmissionGameMode::RemoveSelfGodMode()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	APlayerController* SelfPlayerController = World->GetFirstPlayerController();
	if (SelfPlayerController == nullptr)
	{
		return;
	}

	AWildOmissionCharacter* SelfCharacter = Cast<AWildOmissionCharacter>(SelfPlayerController->GetPawn());
	if (SelfCharacter == nullptr)
	{
		return;
	}

	SelfCharacter->SetGodMode(false);
}

void AWildOmissionGameMode::RemoveAllPlayersGodMode()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController == nullptr)
		{
			return;
		}

		AWildOmissionCharacter* PlayerCharacter = Cast<AWildOmissionCharacter>(PlayerController->GetPawn());
		if (PlayerCharacter == nullptr)
		{
			return;
		}

		PlayerCharacter->SetGodMode(false);
	}
}

void AWildOmissionGameMode::FreezeTime()
{
	if (TimeOfDayManager == nullptr)
	{
		return;
	}

	TimeOfDayManager->SetTimeFrozen(true);
}

void AWildOmissionGameMode::UnfreezeTime()
{
	if (TimeOfDayManager == nullptr)
	{
		return;
	}

	TimeOfDayManager->SetTimeFrozen(false);
}

void AWildOmissionGameMode::SetTime(float NormalizedTime)
{
	if (TimeOfDayManager == nullptr)
	{
		return;
	}

	TimeOfDayManager->SetNormalizedProgressThroughDay(NormalizedTime);
}

void AWildOmissionGameMode::Weather(const FString& WeatherToSet)
{
	if (WeatherManager == nullptr)
	{
		return;
	}

	AStorm* SpawnedStorm = WeatherManager->SpawnStorm(true);
	
	APawn* HostPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (WeatherToSet == TEXT("Rain"))
	{
		FVector NewStormLocation = SpawnedStorm->GetActorLocation();
		NewStormLocation.X = HostPlayer->GetActorLocation().X - 10000.0f;
		NewStormLocation.Y = HostPlayer->GetActorLocation().Y;
		SpawnedStorm->SetActorLocation(NewStormLocation);
		SpawnedStorm->SetMovementVector(FVector(1.0f, 0.0f, 0.0f));
		SpawnedStorm->SetSeverity(30.0f);
	}
	else if (WeatherToSet == TEXT("Tornado"))
	{
		FVector NewStormLocation = SpawnedStorm->GetActorLocation();
		NewStormLocation.X = HostPlayer->GetActorLocation().X - 10000.0f;
		NewStormLocation.Y = HostPlayer->GetActorLocation().Y;
		SpawnedStorm->SetActorLocation(NewStormLocation);
		SpawnedStorm->SetMovementVector(FVector(1.0f, 0.0f, 0.0f));
		SpawnedStorm->SetSeverity(90.0f);
	}
	else if (WeatherToSet == TEXT("Clear"))
	{
		WeatherManager->ClearStorm();
	}
}

void AWildOmissionGameMode::GiveAllPlayersAchievement(const FString& AchievementID)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController == nullptr)
		{
			continue;
		}

		UWOInGameAchievementsComponent* AchievementsComponent = PlayerController->FindComponentByClass<UWOInGameAchievementsComponent>();
		if (AchievementsComponent == nullptr)
		{
			continue;
		}

		AchievementsComponent->UnlockAchievement(AchievementID);
	}
}

ASaveManager* AWildOmissionGameMode::GetSaveManager() const
{
	return SaveManager;
}

AChunkManager* AWildOmissionGameMode::GetChunkManager() const
{
	return ChunkManager;
}

AWeatherManager* AWildOmissionGameMode::GetWeatherManager() const
{
	return WeatherManager;
}

void AWildOmissionGameMode::LogPlayerInventoryComponents()
{
	// Get all the players in the current world
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController == nullptr)
		{
			return;
		}
		
		AWildOmissionCharacter* Character = Cast<AWildOmissionCharacter>(PlayerController->GetPawn());
		if (Character == nullptr)
		{
			return;
		}
		for (const FInventoryItem& ItemData : Character->GetInventoryComponent()->GetContents()->Contents)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.0f, FColor::Orange, FString::Printf(TEXT("%s: %i"), *ItemData.Name.ToString(), ItemData.Quantity));
		}
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.0f, FColor::Green, FString::Printf(TEXT("Player: "), *Character->GetActorNameOrLabel()));
	}
}

void AWildOmissionGameMode::LogPlayerInventorySlots()
{
	// Get all the players in the current world
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController == nullptr)
		{
			return;
		}

		AWildOmissionCharacter* Character = Cast<AWildOmissionCharacter>(PlayerController->GetPawn());
		if (Character == nullptr)
		{
			return;
		}
		for (const FInventorySlot& Slot : Character->GetInventoryComponent()->GetSlots())
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.0f, FColor::Orange, FString::Printf(TEXT("Index: %i, Item: %s, Quantity: %i"), Slot.Index, *Slot.Item.Name.ToString(), Slot.Item.Quantity));
		}
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.0f, FColor::Green, FString::Printf(TEXT("Player: "), *Character->GetActorNameOrLabel()));
	}
}

void AWildOmissionGameMode::ProcessMultiplayerJoinAchievement(AWildOmissionPlayerController* NewWildOmissionPlayer)
{
	UWorld* World = GetWorld();
	if (World == nullptr || NewWildOmissionPlayer == nullptr || NewWildOmissionPlayer->IsLocalPlayerController())
	{
		return;
	}

	APlayerState* NewPlayerState = NewWildOmissionPlayer->GetPlayerState<APlayerState>();
	UAchievementsManager* AchievementsManager = UAchievementsManager::GetAchievementsManager();
	if (NewPlayerState == nullptr || AchievementsManager == nullptr)
	{
		return;
	}

	AchievementsManager->UnlockAchievement(TEXT("ACH_I_HAVE_FRIENDS"));

	APlayerController* HostPlayerController = World->GetFirstPlayerController();
	if (HostPlayerController == nullptr)
	{
		return;
	}

	APlayerState* HostPlayerState = HostPlayerController->GetPlayerState<APlayerState>();
	if (HostPlayerState == nullptr)
	{
		return;
	}

	const FString LarchUniqueId = TEXT("76561198277223961");
	const FString LifeUniqueId  = TEXT("76561198242206838");
	const FString TyceUniqueId  = TEXT("76561199063735949");

	const FString NewPlayerUniqueId = NewPlayerState->GetUniqueId().ToString();
	const FString HostPlayerUniqueId = HostPlayerState->GetUniqueId().ToString();

	if (HostPlayerUniqueId == LarchUniqueId || NewPlayerUniqueId == LarchUniqueId)
	{
		GiveAllPlayersAchievement(TEXT("ACH_IM_THE_REAL_LARCH"));
	}
	else if (HostPlayerUniqueId == TyceUniqueId || NewPlayerUniqueId == TyceUniqueId)
	{
		GiveAllPlayersAchievement(TEXT("ACH_IM_THE_REAL_TYCE"));
	}

	//else if (HostPlayerUniqueId == LifeUniqueId || NewPlayerUniqueId == LifeUniqueId)
	//{
		//GiveAllPlayersAchievement(TEXT("ACH_IM_THE_REAL_LIFE"));
	//}
}

void AWildOmissionGameMode::SpawnHumanAtStartSpot(AController* Controller)
{
	if (ChunkManager == nullptr)
	{
		return;
	}

	const FVector SpawnLocation = ChunkManager->GetWorldSpawnPoint();

	// Spawn there
	if (MustSpectate(Cast<APlayerController>(Controller)))
	{
		UE_LOG(LogGameMode, Verbose, TEXT("RestartPlayerAtPlayerStart: Tried to restart a spectator-only player!"));
		return;
	}

	if (HumanCharacterClass != nullptr)
	{
		// Try to create a pawn to use of the default class for this player
		APawn* NewPawn = GetWorld()->SpawnActor<APawn>(HumanCharacterClass, SpawnLocation + FVector(0.0f, 0.0f, 100.0f), FRotator::ZeroRotator);
		if (IsValid(NewPawn))
		{
			Controller->SetPawn(NewPawn);
		}
	}

	if (!IsValid(Controller->GetPawn()))
	{
		FailedToRestartPlayer(Controller);
	}
	else
	{
		FinishRestartPlayer(Controller, FRotator::ZeroRotator);
	}
}

void AWildOmissionGameMode::SpawnHumanAtBed(AController* Controller)
{
	AWildOmissionPlayerController* WOPlayerController = Cast<AWildOmissionPlayerController>(Controller);
	if (WOPlayerController == nullptr)
	{
		SpawnHumanAtStartSpot(Controller);
		return;
	}
	
	AChunk* BedChunk = ChunkManager->GetChunkAtLocation(WOPlayerController->GetBedChunkLocation());
	if (BedChunk == nullptr)
	{
		return;
	}

	TArray<AActor*> ChunkAttachedActors;
	BedChunk->GetAttachedActors(ChunkAttachedActors);

	TArray<ABed*> ChunkBeds;
	for (AActor* AttachedActor : ChunkAttachedActors)
	{
		ABed* AttachedBed = Cast<ABed>(AttachedActor);
		if (AttachedBed == nullptr)
		{
			continue;
		}

		ChunkBeds.Add(AttachedBed);
	}

	ABed* SpawnBed = nullptr;
	for (ABed* Bed : ChunkBeds)
	{
		if (Bed == nullptr)
		{
			continue;
		}

		if (Bed->GetUniqueID() == WOPlayerController->GetBedUniqueID())
		{
			SpawnBed = Bed;
			break;
		}
	}

	if (SpawnBed == nullptr)
	{
		SpawnHumanAtStartSpot(Controller);
		return;
	}

	const FVector SpawnLocation = SpawnBed->GetSpawnPointComponent()->GetComponentLocation() + FVector(0.0f, 0.0f, 100.0f);
	const FRotator SpawnRotation = SpawnBed->GetSpawnPointComponent()->GetComponentRotation();

	if (HumanCharacterClass != nullptr)
	{
		// Try to create a pawn to use of the default class for this player
		APawn* NewPawn = GetWorld()->SpawnActor<APawn>(HumanCharacterClass, SpawnLocation, SpawnRotation);
		if (IsValid(NewPawn))
		{
			Controller->SetPawn(NewPawn);
		}
	}

	if (!IsValid(Controller->GetPawn()))
	{
		FailedToRestartPlayer(Controller);
	}
	else
	{
		FinishRestartPlayer(Controller, SpawnRotation);
	}
}
