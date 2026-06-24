// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimeOfDayManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimeSunriseSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimeNoonSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimeSunsetSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimeMidnightSignature);

class ADirectionalLight;
UCLASS()
class TIMEOFDAY_API ATimeOfDayManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATimeOfDayManager();

	static void SetTimeOfDayManager(ATimeOfDayManager* NewInstance);
	static ATimeOfDayManager* GetTimeOfDayManager();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetDaysPlayed(int32 InDaysPlayed);
	int32 GetDaysPlayed() const;

	// Sets the time, time is normalized (0 is morning, .25 is noon, .5 is dusk, .75 is midnight)
	void SetTimeOfDay(float InNormalizedTime);

	// Returns a normalized value of the time (0 is morning, .25 is noon, .5 is dusk, .75 is midnight)
	float GetTimeOfDay() const;
	
	void SetTimeFrozen(bool InTimeFrozen);
	bool IsTimeFrozen() const;

	FOnTimeSunriseSignature OnTimeSunrise;
	FOnTimeNoonSignature OnTimeNoon;
	FOnTimeSunsetSignature OnTimeSunset;
	FOnTimeMidnightSignature OnTimeMidnight;

	UFUNCTION(BlueprintCallable)
	bool IsDay() const;
	UFUNCTION(BlueprintCallable)
	bool IsNight() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	UPROPERTY()
	ADirectionalLight* DirectionalLight;
	
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_DaysPlayed)
	int32 DaysPlayed;
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_TimeOfDay)
	float TimeOfDay;

	void CalculateMoonPhase();

	UFUNCTION()
	void OnRep_DaysPlayed();

	UFUNCTION()
	void OnRep_TimeOfDay();
	
	bool TimeFrozen;

	bool SunriseBroadcasted;
	bool NoonBroadcasted;
	bool SunsetBroadcasted;
	bool MidnightBroadcasted;

	UFUNCTION()
	void HandleDelegates();

};
