// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SpecialEffectsManagerComponent.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class UExponentialHeightFogComponent;
class ATimeOfDayManager;
class UCameraComponent;
class AStorm;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WILDOMISSIONCORE_API USpecialEffectsManagerComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USpecialEffectsManagerComponent();
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly)
	UNiagaraSystem* RainParticleSystem;

private:
	UPROPERTY()
	UNiagaraComponent* SpawnedRainComponent;

	UPROPERTY()
	UAudioComponent* SpawnedRainAudioComponent;

	UPROPERTY()
	USoundBase* RainSound;

	UPROPERTY()
	UExponentialHeightFogComponent* FogComponent;

	UPROPERTY()
	ATimeOfDayManager* TimeOfDayManager;

	UPROPERTY()
	UCameraComponent* OwnerCamera;

	UPROPERTY()
	float LowHealthEffectThreshold;

	UPROPERTY()
	AStorm* PreviouslyHitStorm;

	float RainSoundCutoff;

	float NightGammaStrength;

	void HandleNightTimeGamma();
	void HandleLowHealthEffects();
	void HandleWeatherEffects();

	void EnableRainfallEffects(float RainDensity);
	void DisableRainfallEffects();

	AStorm* CastToStorm(AActor* InActor);
	bool LineTraceIntoSkyOnChannel(ECollisionChannel ChannelToTrace, FHitResult& OutHitResult) const;

	UFUNCTION()
	void OnOwnerDestroyed(AActor* DestroyedActor);

};
