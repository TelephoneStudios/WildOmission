// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MusicPlayerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEMUSIC_API UMusicPlayerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMusicPlayerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	UAudioComponent* AudioComponent;

	UPROPERTY(EditDefaultsOnly)
	float TimeBetweenSongs;

	UFUNCTION()
	void PlayMusicTrack();
	
};
