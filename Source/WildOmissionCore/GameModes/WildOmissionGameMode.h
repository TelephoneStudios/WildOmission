// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WildOmissionGameMode.generated.h"

UCLASS()
class WILDOMISSIONCORE_API AWildOmissionGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	virtual void StartPlay() override;

	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	void SpawnHumanForController(APlayerController* Controller);

	UFUNCTION(BlueprintCallable, Exec)
	void SaveWorld();

	UFUNCTION(BlueprintCallable, Exec)
	void TeleportAllPlayersToSelf();

	UFUNCTION(BlueprintCallable, Exec)
	void GiveSelfGodMode();

	UFUNCTION(BlueprintCallable, Exec)
	void GiveAllPlayersGodMode();

	UFUNCTION(BlueprintCallable, Exec)
	void RemoveSelfGodMode();

	UFUNCTION(BlueprintCallable, Exec)
	void RemoveAllPlayersGodMode();

	UFUNCTION(BlueprintCallable, Exec)
	void FreezeTime();

	UFUNCTION(BlueprintCallable, Exec)
	void UnfreezeTime();

	UFUNCTION(BlueprintCallable, Exec)
	void SetTime(float NormalizedTime);

	UFUNCTION(BlueprintCallable, Exec)
	void Weather(const FString& WeatherToSet);

	UFUNCTION(BlueprintCallable, Exec)
	void GiveAllPlayersAchievement(const FString& AchievementID);

	class ASaveManager* GetSaveManager() const;
	class AChunkManager* GetChunkManager() const;
	class AWeatherManager* GetWeatherManager() const;

	UFUNCTION(Exec)
	void LogPlayerInventoryComponents();

	UFUNCTION(Exec)
	void LogPlayerInventorySlots();

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APawn> HumanCharacterClass;

	bool FriendsOnly = true;

	uint8 GameMode = 0;

	UPROPERTY()
	class ASaveManager* SaveManager;

	UPROPERTY()
	class AChunkManager* ChunkManager;
	
	UPROPERTY()
	class ATimeOfDayManager* TimeOfDayManager;
	
	UPROPERTY()
	class AWeatherManager* WeatherManager;

	UPROPERTY()
	class AAnimalSpawnManager* AnimalSpawnManager;

	UPROPERTY()
	class AMonsterSpawnManager* MonsterSpawnManager;
	
	UPROPERTY()
	class AGameChatManager* ChatManager;

	void ProcessMultiplayerJoinAchievement(class AWildOmissionPlayerController* NewWildOmissionPlayer);

	void SpawnHumanAtStartSpot(AController* Controller);
	void SpawnHumanAtBed(AController* Controller);

};
