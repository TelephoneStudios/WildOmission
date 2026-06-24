// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/SavablePlayer.h"
#include "Interfaces/PlayerControllerMessageSender.h"
#include "Interfaces/BedController.h"
#include "Interfaces/MapInterface.h"
#include "Interfaces/ServerAdministrator.h"
#include "WildOmissionPlayerController.generated.h"

UCLASS()
class WILDOMISSIONCORE_API AWildOmissionPlayerController 
	: public APlayerController, public ISavablePlayer, public IPlayerControllerMessageSender, public IBedController, public IMapInterface, public IServerAdministrator
{
	GENERATED_BODY()

public:
	AWildOmissionPlayerController();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Begin ISavablePlayer Implementation
	virtual FPlayerSaveData SavePlayer() override;
	virtual void LoadPlayerSave(const FPlayerSaveData& SaveData) override;
	virtual bool IsStillLoading() const override;
	virtual FString GetUniqueID() const override;
	virtual bool IsHost() const override;
	// End ISavablePlayer Implementation

	// Begin IPlayerControllerMessageSender Implementation
	virtual void SendMessage(APlayerState* Sender, const FString& Message) override;
	// End IPlayerControllerMessageSender Implementation

	// Begin IBedController Implementation
	virtual void Sleep() override;
	virtual void SetCurrentBed(const int32& InBedUniqueID, const FIntVector2& InBedChunkLocation) override;
	virtual int32 GetBedUniqueID() const override;
	virtual FIntVector2 GetBedChunkLocation() const override;
	// End IBedController Implementation

	// Begin IServerAdministrator Implementation
	virtual void SetAdministrator(bool InAdministrator) override;
	virtual bool IsAdministrator() const override;
	virtual void KickPlayer(APlayerController* PlayerControllerToKick) override;
	// End IServerAdministrator Implementation

	UFUNCTION(BlueprintCallable)
	void SetGameModeIndex(const uint8 & NewGameModeIndex);
	UFUNCTION(BlueprintCallable)
	uint8 GetGameModeIndex() const;

	UFUNCTION(BlueprintCallable)
	void SetInCheatedWorld(bool CheatedWorld);
	UFUNCTION(BlueprintCallable)
	bool IsInCheatedWorld() const;


	UFUNCTION(BlueprintCallable)
	bool IsSurvivalMode() const;
	UFUNCTION(BlueprintCallable)
	bool IsCreativeMode() const;

	void Save();

	UFUNCTION(BlueprintCallable)
	bool IsEditorPlayer() const;

	UFUNCTION(Client, Reliable)
	void Client_ShowDeathMenu();

	UFUNCTION(Server, Reliable)
	void Server_Spawn();

	UFUNCTION()
	void OnPlayerDeath(const FVector& DeathLocation);

	//*****************************
	// Console functions
	UFUNCTION(Exec)
	void Kill();

	// This isn't really used anymore
	UFUNCTION(Exec)
	void LogLocalInventoryContents();

	virtual void SetSavedLocationToCurrentLocation() override;
	virtual void SetLastDeathLocation(const FVector& DeathLocation) override;
	virtual FVector GetCurrentCoordinateLocation() override;
	virtual FVector GetSavedLocation() override;
	virtual FVector GetLastDeathLocation() override;

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* aPawn) override;

private:
	UPROPERTY(VisibleAnywhere)
	class UMusicPlayerComponent* MusicPlayerComponent;

	UPROPERTY(VisibleAnywhere)
	class UWOInGameAchievementsComponent* AchievementsComponent;

	UPROPERTY()
	TSubclassOf<class UDeathMenuWidget> DeathMenuWidgetClass;

	UPROPERTY(VisibleAnywhere, Replicated)
	uint8 GameModeIndex;

	UPROPERTY(VisibleAnywhere, Replicated)
	bool InCheatedWorld;

	UPROPERTY(VisibleAnywhere, Replicated)
	int32 BedUniqueID;

	UPROPERTY(VisibleAnywhere)
	FIntVector2 BedChunkLocation;

	UPROPERTY(Replicated)
	FVector LastDeathLocation;

	UPROPERTY(Replicated)
	FVector SavedLocation;

	UPROPERTY(Replicated)
	bool Administrator;

	UPROPERTY()
	class AChunkInvokerActor* TempChunkInvoker;

	UPROPERTY(Replicated)
	FIntVector2 SpawnChunk;

	FTimerHandle CheckSpawnChunkValidTimerHandle;

	void SetupPlayerOnServer();
	void StartLoading();
	void SetupMusicPlayerComponent();
	void SetupLocalAsHost();
	void SetupLocalAsClient();

	UFUNCTION()
	void CheckSpawnChunkValid();
	
	void StopLoading();

	// Used by the server to figure out if a player is still loading
	// into the server, it will wait until the player if fully loaded before attempting
	// to load their data from the save file.
	bool bIsStillLoading;
	FPlayerSaveData StoredPlayerSaveData;

	UFUNCTION(Server, Reliable)
	void Server_SendMessage(APlayerState* Sender, const FString& Message);

	UFUNCTION(Server, Reliable)
	void Server_AddToPendingSaves();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_KickPlayer(APlayerController* PlayerControllerToKick);

	UFUNCTION(Server, Reliable)
	void Server_KillThisPlayer();

	UFUNCTION(Server, Reliable)
	void Server_SetSavedLocationToCurrentLocation();

};
