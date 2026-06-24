// Copyright Telephone Studios. All Rights Reserved.


#include "WeatherManager.h"
#include "Actors/Storm.h"
#include "Actors/Tornado.h"
#include "Structs/WindParameters.h"
#include "TimeOfDayManager.h"
#include "Components/WeatherSaveComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

static AWeatherManager* Instance = nullptr;
static UMaterialParameterCollection* MPC_WindCollection = nullptr;

// Sets default values
AWeatherManager::AWeatherManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;
	NetUpdateFrequency = 2.0f;
	
	SaveComponent = CreateDefaultSubobject<UWeatherSaveComponent>(TEXT("SaveComponent"));

	StormsDisabled = false;

	CurrentStorm = nullptr;
	StormSpawnChances = TArray<float>
	{
		0.1f,	// SunriseStormSpawnChance 
		0.25f,	// NoonStormSpawnChance
		0.3f,	// SunsetStormSpawnChance 
		0.2f	// MidnightStormSpawnChance 
	};

	static ConstructorHelpers::FClassFinder<AStorm> StormBlueprint(TEXT("/Game/Weather/Actors/BP_Storm"));
	if (StormBlueprint.Succeeded())
	{
		StormClass = StormBlueprint.Class;
	}
	static ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> WindParameterCollectionBlueprint(TEXT("/Game/Weather/MPC_Wind"));
	if (WindParameterCollectionBlueprint.Succeeded())
	{
		MPC_WindCollection = WindParameterCollectionBlueprint.Object;
	}
}

void AWeatherManager::SetWeatherManager(AWeatherManager* NewInstance)
{
	Instance = NewInstance;
}

AWeatherManager* AWeatherManager::GetWeatherManager()
{
	return Instance;
}

// Called when the game starts or when spawned
void AWeatherManager::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (World == nullptr || World->IsEditorWorld() && IsValid(Instance))
	{
		return;
	}

	SetWeatherManager(this);

	if (!HasAuthority())
	{
		return;
	}
	
	ATimeOfDayManager* TimeOfDayManager = ATimeOfDayManager::GetTimeOfDayManager();
	if (TimeOfDayManager == nullptr)
	{
		return;
	}

	TimeOfDayManager->OnTimeSunrise.AddDynamic(this, &AWeatherManager::AttemptToSpawnStorm);
	TimeOfDayManager->OnTimeNoon.AddDynamic(this, &AWeatherManager::AttemptToSpawnStorm);
	TimeOfDayManager->OnTimeSunset.AddDynamic(this, &AWeatherManager::AttemptToSpawnStorm);
	TimeOfDayManager->OnTimeMidnight.AddDynamic(this, &AWeatherManager::AttemptToSpawnStorm);
}

void AWeatherManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	Instance = nullptr;
}

// Called every frame
void AWeatherManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateWindParameters();
}

void AWeatherManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeatherManager, CurrentStorm);
}

void AWeatherManager::Save(FWeatherData& OutWeatherData)
{
	if (SaveComponent == nullptr)
	{
		return;
	}

	SaveComponent->Save(OutWeatherData);
}

void AWeatherManager::Load(const FWeatherData& InWeatherData)
{
	if (SaveComponent == nullptr)
	{
		return;
	}

	SaveComponent->Load(InWeatherData);
}

AStorm* AWeatherManager::SpawnStorm(bool FromCommand)
{
	UWorld* World = GetWorld();
	if (World == nullptr || StormsDisabled || StormClass == nullptr)
	{
		return nullptr;
	}

	if (CurrentStorm)
	{
		return CurrentStorm;
	}

	CurrentStorm = World->SpawnActor<AStorm>(StormClass);
	CurrentStorm->Setup(FromCommand);
	CurrentStorm->OnCleanup.AddDynamic(this, &AWeatherManager::OnStormCleanup);

	return CurrentStorm;
}

void AWeatherManager::ClearStorm()
{
	if (CurrentStorm == nullptr)
	{
		return;
	}

	CurrentStorm->Cleanup();
}

void AWeatherManager::SetCurrentStorm(AStorm* NewCurrentStorm)
{
	if (CurrentStorm)
	{
		CurrentStorm->Cleanup();
	}

	CurrentStorm = NewCurrentStorm;
}

void AWeatherManager::SetStormsDisabled(bool InStormsDisabled)
{
	StormsDisabled = InStormsDisabled;
}

void AWeatherManager::AttemptToSpawnStorm()
{
	ATimeOfDayManager* TimeOfDayManager = ATimeOfDayManager::GetTimeOfDayManager();
	if (TimeOfDayManager == nullptr)
	{
		return;
	}

	const float SpawnChance = StormSpawnChances[FMath::RoundToInt32(TimeOfDayManager->GetTimeOfDay() / 0.25f)];

	const bool ShouldSpawn = UKismetMathLibrary::RandomBoolWithWeight(SpawnChance);
	if (ShouldSpawn == false)
	{
		return;
	}

	SpawnStorm();
}

void AWeatherManager::OnStormCleanup()
{
	CurrentStorm = nullptr;
}

void AWeatherManager::UpdateWindParameters()
{
	FWindParameters NewParams;

	if (CurrentStorm == nullptr)
	{
		ApplyWindParameters(NewParams);
		return;
	}

	float RemainingDistance = CurrentStorm->GetTravelDistance() - CurrentStorm->GetDistanceTraveled();
	float DistanceFadeRatio = CurrentStorm->GetTravelDistance() * 0.2f;

	float NormalizedSeverity = CurrentStorm->GetSeverity() / 100.0f;

	float FadeInOutMultiplier = 1.0f;
	if (CurrentStorm->GetDistanceTraveled() < DistanceFadeRatio)
	{
		FadeInOutMultiplier = CurrentStorm->GetDistanceTraveled() / 1000.0f;
	}
	else if (RemainingDistance < DistanceFadeRatio)
	{
		FadeInOutMultiplier = (1000.0f / RemainingDistance) - 1.0f;
	}

	NewParams.GlobalWindStrength = FMath::Clamp(NormalizedSeverity * FadeInOutMultiplier, 0.3f, 1.0f);
	NewParams.GlobalWindDirection = FVector2D(CurrentStorm->GetMovementVector().X, CurrentStorm->GetMovementVector().Y);

	// If the wind direction is zero then trees will vanish from sight
	if (NewParams.GlobalWindDirection.IsNearlyZero(KINDA_SMALL_NUMBER))
	{
		NewParams.GlobalWindDirection = FVector2D(0.5f, 0.5f);
	}

	NewParams.TornadoOnGround = 0.0f;
	NewParams.TornadoLocation = FVector2D::ZeroVector;

	ATornado* SpawnedTornado = CurrentStorm->GetSpawnedTornado();
	if (SpawnedTornado)
	{
		NewParams.TornadoLocation = FVector2D(SpawnedTornado->GetActorLocation().X, SpawnedTornado->GetActorLocation().Y);
		NewParams.TornadoOnGround = 1.0f;
	}

	ApplyWindParameters(NewParams);
}

void AWeatherManager::ApplyWindParameters(const FWindParameters& NewParameters)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	UKismetMaterialLibrary::SetScalarParameterValue(World, MPC_WindCollection, TEXT("GlobalWindStrength"), NewParameters.GlobalWindStrength);
	UKismetMaterialLibrary::SetVectorParameterValue(World, MPC_WindCollection, TEXT("GlobalWindDirection"), FLinearColor(NewParameters.GlobalWindDirection.X, NewParameters.GlobalWindDirection.Y, 0.0f, 1.0f));
	UKismetMaterialLibrary::SetScalarParameterValue(World, MPC_WindCollection, TEXT("TornadoOnGround"), NewParameters.TornadoOnGround);
	UKismetMaterialLibrary::SetVectorParameterValue(World, MPC_WindCollection, TEXT("TornadoLocation"), FLinearColor(NewParameters.TornadoLocation.X, NewParameters.TornadoLocation.Y, 0.0f, 1.0f));
}

AStorm* AWeatherManager::GetCurrentStorm() const
{
	return CurrentStorm;
}

bool AWeatherManager::CanSpawnStorm() const
{
	return !StormsDisabled && CurrentStorm == nullptr;
}

bool AWeatherManager::GetStormsDisabled() const
{
	return StormsDisabled;
}