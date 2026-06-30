// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SaveManager.generated.h"

UCLASS()
class SAVESYSTEM_API ASaveManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASaveManager();

	void SetGameSaveLoadController(class IGameSaveLoadController* InGameSaveLoadController);

	static void SetSaveManager(ASaveManager* NewInstance);
	static ASaveManager* GetSaveManager();

	void SaveWorld();
	void SetWorld(const FString& WorldName);
	void LoadWorld();

	void UpdateWorldFile(class UWorldInformation* UpdatedWorldInformation, class UWorldData* UpdatedWorldData);
	void UpdateWorldInformation(class UWorldInformation* UpdatedWorldInformation);
	void UpdateWorldData(class UWorldData* UpdatedWorldData);

	void CaptureWorldIcon();
	static UTexture2D* GetWorldIcon(const FString& WorldName);

	class UPlayerSaveManagerComponent* GetPlayerManager() const;

	class UWorldInformation* GetWorldInformation() const;
	class UWorldData* GetWorldData() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	FString CurrentWorldName;

	UPROPERTY(VisibleAnywhere)
	class UPlayerSaveManagerComponent* PlayerSaveManagerComponent;

	class IGameSaveLoadController* GameSaveLoadController;

	void ValidateSave();

	UPROPERTY()
	class UWorldData* CurrentWorldData;

	UPROPERTY()
	class UWorldInformation* CurrentWorldInformation;

	UFUNCTION()
	void StartLoading();

	UFUNCTION()
	void StopLoading();

	UFUNCTION()
	void SetLoadingTitle(const FString& NewTitle);

	UFUNCTION()
	void SetLoadingSubtitle(const FString& NewSubtitle);

	UFUNCTION()
	void CreateWorld(const FString& NewWorldName);

	bool DoesWorldAlreadExist(const FString& WorldName) const;
};