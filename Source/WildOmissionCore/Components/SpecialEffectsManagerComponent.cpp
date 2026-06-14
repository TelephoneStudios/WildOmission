// Copyright Telephone Studios. All Rights Reserved.


#include "SpecialEffectsManagerComponent.h"
#include "Actors/Storm.h"
#include "Components/VitalsComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Camera/CameraComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "TimeOfDayManager.h"
#include "WildOmissionGameUserSettings.h"
#include "AchievementsManager.h"
#include "Components/PostProcessComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"

static UMaterialParameterCollection* MPC_Effects = nullptr;

// Sets default values for this component's properties
USpecialEffectsManagerComponent::USpecialEffectsManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	RainParticleSystem = nullptr;

	SpawnedRainComponent = nullptr;
	SpawnedRainAudioComponent = nullptr;
	RainSound = nullptr;
	FogComponent = nullptr;
	TimeOfDayManager = nullptr;
	OwnerCamera = nullptr;

	PreviouslyHitStorm = nullptr;
	
	RainSoundCutoff = 20000.0f;
	NightGammaStrength = 0.0f;
	LowHealthEffectThreshold = 60.0f;

	static ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> EffectsParameterCollection(TEXT("/Game/WildOmissionCore/Art/Environment/MPC_Effects"));
	if (EffectsParameterCollection.Succeeded())
	{
		MPC_Effects = EffectsParameterCollection.Object;
	}

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> RainSystemBlueprint(TEXT("/Game/Weather/Art/NS_Rain"));
	if (RainSystemBlueprint.Succeeded())
	{
		RainParticleSystem = RainSystemBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMetaSoundSource> RainSoundBlueprint(TEXT("/Game/Weather/Audio/MS_RainFall"));
	if (RainSoundBlueprint.Succeeded())
	{
		RainSound = RainSoundBlueprint.Object;
	}
}

// Called when the game starts
void USpecialEffectsManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwner();
	if (World == nullptr || OwnerActor == nullptr)
	{
		return;
	}

	OwnerActor->OnDestroyed.AddDynamic(this, &USpecialEffectsManagerComponent::OnOwnerDestroyed);

	OwnerCamera = OwnerActor->FindComponentByClass<UCameraComponent>();

	AActor* HeightFogActor = UGameplayStatics::GetActorOfClass(World, AExponentialHeightFog::StaticClass());
	if (HeightFogActor)
	{
		FogComponent = HeightFogActor->FindComponentByClass<UExponentialHeightFogComponent>();
	}


	TimeOfDayManager = ATimeOfDayManager::GetTimeOfDayManager();
}


// Called every frame
void USpecialEffectsManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	HandleNightTimeGamma();
	HandleLowHealthEffects();
	HandleWeatherEffects();
}

void USpecialEffectsManagerComponent::HandleNightTimeGamma()
{
	UWorld* World = GetWorld();
	if (World == nullptr || OwnerCamera == nullptr)
	{
		return;
	}
	
	if (TimeOfDayManager->IsNight())
	{
		NightGammaStrength = FMath::Clamp(NightGammaStrength + (0.1f * World->GetDeltaSeconds()), 0.0f, 1.0f);
	}
	else
	{
		NightGammaStrength = FMath::Clamp(NightGammaStrength - (0.1f * World->GetDeltaSeconds()), 0.0f, 1.0f);
	}
	
	bool AutoExposureEnabled = false;
	UWildOmissionGameUserSettings* UserSettings = UWildOmissionGameUserSettings::GetWildOmissionGameUserSettings();
	if (UserSettings)
	{
		AutoExposureEnabled = UserSettings->GetAutoExposureEnabled();

		float GammaAddition = FMath::Lerp(0.0f, 0.3f, NightGammaStrength);
		float ColorGammaValue = (UserSettings->GetGamma() / 100.0f) + GammaAddition;
		OwnerCamera->PostProcessSettings.ColorGamma = FVector4(ColorGammaValue, ColorGammaValue, ColorGammaValue, ColorGammaValue);
	}
	
	// process exposure bias differently if auto exposure is turned off
	// so it's not pitch black at night
	if (AutoExposureEnabled)
	{
		OwnerCamera->PostProcessSettings.AutoExposureBias = FMath::Lerp(0.5f, -4.0f, NightGammaStrength);
	}
	else
	{
		OwnerCamera->PostProcessSettings.AutoExposureBias = FMath::Lerp(1.0f, 4.0f, NightGammaStrength);
	}
}

void USpecialEffectsManagerComponent::HandleLowHealthEffects()
{
	AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr || OwnerCamera == nullptr)
	{
		return;
	}

	UVitalsComponent* OwnerVitalsComponent = OwnerActor->FindComponentByClass<UVitalsComponent>();
	if (OwnerVitalsComponent == nullptr)
	{
		return;
	}

	float ContrastAmount = FMath::Lerp(0.7f, 1.0f, FMath::Clamp(OwnerVitalsComponent->GetHealth() / LowHealthEffectThreshold, 0.0f, 1.0f));
	
	OwnerCamera->PostProcessSettings.SceneFringeIntensity = FMath::Lerp(5.0f, 0.0f, FMath::Clamp(OwnerVitalsComponent->GetHealth() / LowHealthEffectThreshold, 0.0f, 1.0f));
	OwnerCamera->PostProcessSettings.ColorContrast = FVector4(ContrastAmount, ContrastAmount, ContrastAmount, ContrastAmount);
	OwnerCamera->PostProcessSettings.BloomDirtMaskIntensity = FMath::Lerp(200.0f, 0.0f, FMath::Clamp(OwnerVitalsComponent->GetHealth() / LowHealthEffectThreshold, 0.0f, 1.0f));
}

void USpecialEffectsManagerComponent::HandleWeatherEffects()
{
	FHitResult HitResult;
	if (!LineTraceIntoSkyOnChannel(ECollisionChannel::ECC_GameTraceChannel2, HitResult))
	{
		DisableRainfallEffects();
		return;
	}


	AStorm* HitStorm = CastToStorm(HitResult.GetActor());
	if (HitStorm == nullptr)
	{
		DisableRainfallEffects();
		return;
	}

	PreviouslyHitStorm = HitStorm;

	float RainDensity = 0.0f;
	if (!HitStorm->IsRaining(RainDensity))
	{
		DisableRainfallEffects();
		return;
	}

	EnableRainfallEffects(RainDensity);
}

void USpecialEffectsManagerComponent::EnableRainfallEffects(float RainDensity)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	if (FogComponent)
	{
		FogComponent->SetFogDensity(0.05f);
		FogComponent->SetFogHeightFalloff(0.001f);
	}

	UKismetMaterialLibrary::SetScalarParameterValue(World, MPC_Effects, TEXT("FogIntensity"), RainDensity / 2200.0f);
	
	if (SpawnedRainComponent == nullptr)
	{
		SpawnedRainComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(RainParticleSystem, this, NAME_None, FVector(0.0f, 0.0f, 1000.0f), FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
	}
	SpawnedRainComponent->SetFloatParameter(TEXT("RainDensity"), RainDensity);

	if (SpawnedRainAudioComponent == nullptr)
	{
		SpawnedRainAudioComponent = UGameplayStatics::SpawnSoundAttached(RainSound, this);
	}
	SpawnedRainAudioComponent->SetFloatParameter(TEXT("RainDensity"), RainDensity);

	FHitResult HitResult;
	if (LineTraceIntoSkyOnChannel(ECollisionChannel::ECC_Visibility, HitResult) && CastToStorm(HitResult.GetActor()) == nullptr)
	{
		RainSoundCutoff = FMath::Clamp(FMath::Lerp(RainSoundCutoff, 500.0f, 5.0f * World->GetDeltaSeconds()), 500.0f, 20000.0f);
	}
	else
	{
		RainSoundCutoff = FMath::Clamp(FMath::Lerp(RainSoundCutoff, 20000.0f, 5.0f * World->GetDeltaSeconds()), 500.0f, 20000.0f);
	}

	SpawnedRainAudioComponent->SetFloatParameter(TEXT("Cutoff"), RainSoundCutoff);

	if (PreviouslyHitStorm != nullptr)
	{
		PreviouslyHitStorm->SetLocalPlayerUnderneath(true);
	}

	UAchievementsManager* AchievementsManager = UAchievementsManager::GetAchievementsManager();
	if (AchievementsManager)
	{
		AchievementsManager->UnlockAchievement(TEXT("ACH_ITS_STORMING"));
	}
}

void USpecialEffectsManagerComponent::DisableRainfallEffects()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// Old values were
	// Density = 0.02
	// HeightFalloff = 0.2
	if (FogComponent)
	{
		FogComponent->SetFogDensity(0.005f);
		FogComponent->SetFogHeightFalloff(0.5f);
	}
	
	UKismetMaterialLibrary::SetScalarParameterValue(World, MPC_Effects, TEXT("FogIntensity"), 0.0f);

	if (SpawnedRainComponent)
	{
		SpawnedRainComponent->DestroyComponent();
		SpawnedRainComponent = nullptr;
	}
	if (SpawnedRainAudioComponent)
	{
		SpawnedRainAudioComponent->Stop();
		SpawnedRainAudioComponent->DestroyComponent();
		SpawnedRainAudioComponent = nullptr;
	}
	if (PreviouslyHitStorm)
	{
		PreviouslyHitStorm->SetLocalPlayerUnderneath(false);
		PreviouslyHitStorm = nullptr;
	}
}

AStorm* USpecialEffectsManagerComponent::CastToStorm(AActor* InActor)
{
	return Cast<AStorm>(InActor);
}

bool USpecialEffectsManagerComponent::LineTraceIntoSkyOnChannel(ECollisionChannel ChannelToTrace, FHitResult& OutHitResult) const
{
	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwner();
	if (World == nullptr || OwnerActor == nullptr)
	{
		return false;
	}

	FVector Start = GetComponentLocation();
	FVector End = Start + FVector::UpVector * 100000.0f;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerActor);

	return World->LineTraceSingleByChannel(OutHitResult, Start, End, ChannelToTrace, Params);
}

void USpecialEffectsManagerComponent::OnOwnerDestroyed(AActor* DestroyedActor)
{
	if (SpawnedRainComponent)
	{
		SpawnedRainComponent->DestroyComponent();
		SpawnedRainComponent = nullptr;
	}

	if (SpawnedRainAudioComponent)
	{
		SpawnedRainAudioComponent->Stop();
		SpawnedRainAudioComponent->DestroyComponent();
		SpawnedRainAudioComponent = nullptr;
	}
}
