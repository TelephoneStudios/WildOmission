// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameSaveLoadController.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, NotBlueprintable)
class UGameSaveLoadController : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SAVESYSTEM_API IGameSaveLoadController
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, Exec)
	virtual void StartLoading() = 0;

	UFUNCTION(BlueprintCallable, Exec)
	virtual void StopLoading() = 0;

	UFUNCTION(BlueprintCallable, Exec)
	virtual void SetLoadingTitle(const FString& InLoadingTitle) = 0;

	UFUNCTION(BlueprintCallable, Exec)
	virtual void SetLoadingSubtitle(const FString& InLoadingSubtitle) = 0;

	UFUNCTION(BlueprintCallable, Exec)
	virtual void CreateWorld(const FString& WorldName, const FString& SeedOverride = TEXT("")) = 0;

	UFUNCTION(BlueprintCallable, Exec)
	virtual bool DoesWorldAlreadyExist(const FString& WorldName) const = 0;

	UFUNCTION(BlueprintCallable, Exec)
	virtual void RenameWorld(const FString& OldWorldName, const FString& NewWorldName) = 0;

	UFUNCTION(BlueprintCallable, Exec)
	virtual void DeleteWorld(const FString& WorldName) = 0;

};
