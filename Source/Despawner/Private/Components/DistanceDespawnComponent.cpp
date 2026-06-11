// Copyright Telephone Studios. All Rights Reserved.


#include "Components/DistanceDespawnComponent.h"

const static float CONDITION_CHECK_INTERVAL = 10.0f;

// Sets default values for this component's properties
UDistanceDespawnComponent::UDistanceDespawnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}
