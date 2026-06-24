// Copyright Telephone Studios. All Rights Reserved.


#include "TimeOfDayManager.h"
#include "Engine/DirectionalLight.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Net/UnrealNetwork.h"
#include "Log.h"

static ATimeOfDayManager* Instance = nullptr;
static const float DAY_NIGHT_SPEED = 0.3f;
static UMaterialParameterCollection* MPC_Sky = nullptr;

// Sets default values
ATimeOfDayManager::ATimeOfDayManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;
	NetUpdateFrequency = 1.0f;

	DirectionalLight = nullptr;
	TimeOfDay = 0.0f;
	DaysPlayed = 0;

	TimeFrozen = false;

	SunriseBroadcasted = false;
	NoonBroadcasted = false;
	SunsetBroadcasted = false;
	MidnightBroadcasted = false;

	static ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> SkyCollectionBlueprint(TEXT("/Game/WildOmissionCore/Art/Environment/MPC_Sky"));
	if (SkyCollectionBlueprint.Succeeded())
	{
		MPC_Sky = SkyCollectionBlueprint.Object;
	}
}

void ATimeOfDayManager::SetTimeOfDayManager(ATimeOfDayManager* NewInstance)
{
	Instance = NewInstance;
}

ATimeOfDayManager* ATimeOfDayManager::GetTimeOfDayManager()
{
	return Instance;
}

// Called when the game starts or when spawned
void ATimeOfDayManager::BeginPlay()
{
	Super::BeginPlay();
	

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	DirectionalLight = Cast<ADirectionalLight>(UGameplayStatics::GetActorOfClass(World, ADirectionalLight::StaticClass()));

	if (IsValid(Instance))
	{
		return;
	}

	SetTimeOfDayManager(this);
}

void ATimeOfDayManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	Instance = nullptr;
}

void ATimeOfDayManager::CalculateMoonPhase()
{
	if (MPC_Sky == nullptr)
	{
		UE_LOG(LogTimeOfDay, Error, TEXT("Couldn't calculate moon phase, MPC missing."));
		return;
	}
	
	const float MoonPhase = (static_cast<float>(DaysPlayed) / 8.0f) - FMath::Floor((static_cast<float>(DaysPlayed) / 8.0f));
	UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), MPC_Sky, TEXT("MoonPhase"), MoonPhase);
}

void ATimeOfDayManager::OnRep_DaysPlayed()
{
	SetDaysPlayed(DaysPlayed);
}

void ATimeOfDayManager::OnRep_TimeOfDay()
{
	SetTimeOfDay(TimeOfDay);
	HandleDelegates();
}

void ATimeOfDayManager::HandleDelegates()
{
	// Sunrise = 0, Noon = .25, Sunset = .5, Midnight = .75
	const bool SunriseConditionValid = TimeOfDay > 0 && TimeOfDay < 0.05f;
	const bool NoonConditionValid = TimeOfDay > 0.25f && TimeOfDay < 0.3f;
	const bool SunsetConditionValid = TimeOfDay > 0.5f && TimeOfDay < 0.55f;
	const bool MidnightConditionValid = TimeOfDay > 0.75f && TimeOfDay < 0.8f;
	if (!SunriseBroadcasted && OnTimeSunrise.IsBound() && SunriseConditionValid)
	{
		OnTimeSunrise.Broadcast();
		
		SunriseBroadcasted = true;
		NoonBroadcasted = false;
		SunsetBroadcasted = false;
		MidnightBroadcasted = false;
	}
	else if (!NoonBroadcasted && OnTimeNoon.IsBound() && NoonConditionValid)
	{
		OnTimeNoon.Broadcast();

		SunriseBroadcasted = false;
		NoonBroadcasted = true;
		SunsetBroadcasted = false;
		MidnightBroadcasted = false;
	}
	else if (!SunsetConditionValid && OnTimeSunset.IsBound() && SunsetConditionValid)
	{
		OnTimeSunset.Broadcast();

		SunriseBroadcasted = false;
		NoonBroadcasted = false;
		SunsetBroadcasted = true;
		MidnightBroadcasted = false;
	}
	else if (!MidnightConditionValid && OnTimeMidnight.IsBound() && MidnightConditionValid)
	{
		OnTimeMidnight.Broadcast();

		SunriseBroadcasted = false;
		NoonBroadcasted = false;
		SunsetBroadcasted = false;
		MidnightBroadcasted = true;
	}
}

// Called every frame
void ATimeOfDayManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (TimeFrozen)
	{
		return;
	}

	TimeOfDay += (DAY_NIGHT_SPEED * DeltaTime / 360.0f);
	if (TimeOfDay >= 1.0f)
	{
		DaysPlayed += 1;
		TimeOfDay = 0.0f;
	}

	OnRep_DaysPlayed();
	OnRep_TimeOfDay();
}

void ATimeOfDayManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATimeOfDayManager, DaysPlayed);
	DOREPLIFETIME(ATimeOfDayManager, TimeOfDay);
}

void ATimeOfDayManager::SetDaysPlayed(int32 InDaysPlayed)
{
	DaysPlayed = InDaysPlayed;
	CalculateMoonPhase();
}

int32 ATimeOfDayManager::GetDaysPlayed() const
{
	return DaysPlayed;
}

void ATimeOfDayManager::SetTimeOfDay(float InNormalizedTime)
{
	TimeOfDay = InNormalizedTime;
	
	if (DirectionalLight == nullptr)
	{
		return;
	}

	DirectionalLight->SetActorRotation(FRotator(0.0f, 180.0f, 180.0f));
	DirectionalLight->AddActorLocalRotation(FRotator(TimeOfDay * 360.0f, 0.0f, 0.0f));
}

float ATimeOfDayManager::GetTimeOfDay() const
{
	return TimeOfDay;
}

void ATimeOfDayManager::SetTimeFrozen(bool InTimeFrozen)
{
	TimeFrozen = InTimeFrozen;
}

bool ATimeOfDayManager::IsTimeFrozen() const
{
	return TimeFrozen;
}

bool ATimeOfDayManager::IsDay() const
{
	return !IsNight();
}

bool ATimeOfDayManager::IsNight() const
{
	return TimeOfDay > 0.5f;
}