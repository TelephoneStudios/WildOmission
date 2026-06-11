// Copyright Telephone Studios. All Rights Reserved.


#include "WindSuckerComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/OverlapResult.h"

// Sets default values for this component's properties
UWindSuckerComponent::UWindSuckerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PrePhysics;

	Radius = 8000.0f;
	Falloff = ERadialImpulseFalloff::RIF_Linear;
	ForceStrength = -999999.0f;
	DealsDamageToPawns = false;
	UpdateFrequencySeconds = 0.1f;
	UpdateTime = 0.0f;
	bAutoActivate = true;

	// by default we affect all 'dynamic' objects that can currently be affected by forces
	AddCollisionChannelToAffect(ECC_Pawn);
	AddCollisionChannelToAffect(ECC_PhysicsBody);
	AddCollisionChannelToAffect(ECC_Vehicle);
	AddCollisionChannelToAffect(ECC_Destructible);

	UpdateCollisionObjectQueryParams();
}


// Called when the game starts
void UWindSuckerComponent::BeginPlay()
{
	Super::BeginPlay();

	UpdateCollisionObjectQueryParams();
}

void UWindSuckerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	AActor* Owner = GetOwner();
	if (!IsActive() || Owner == nullptr || !Owner->HasAuthority())
	{
		return;
	}

	UpdateTime += DeltaTime;
	if (UpdateTime < UpdateFrequencySeconds)
	{
		return;
	}

	Update();
	UpdateTime = 0.0f;
}

void UWindSuckerComponent::Update()
{
	const FVector Origin = GetComponentLocation();

	// Find objects within the sphere
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AddForceOverlap), false);
	Params.AddIgnoredActor(GetOwner());

	GetWorld()->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity, CollisionObjectQueryParams, FCollisionShape::MakeSphere(Radius), Params);
	const float EffectiveForceStrength = ForceStrength * (UpdateTime + 10.0f);
	for (const FOverlapResult& Overlap : Overlaps)
	{
		UPrimitiveComponent* PrimitiveComponent = Overlap.Component.Get();
		if (PrimitiveComponent == nullptr)
		{
			continue;
		}

		PrimitiveComponent->AddRadialForce(Origin, Radius, EffectiveForceStrength, Falloff);

		// see if this is a target for a movement component
		AActor* ComponentOwner = PrimitiveComponent->GetOwner();
		if (ComponentOwner == nullptr || !HasLineOfSightToActor(ComponentOwner))
		{
			continue;
		}

		UMovementComponent* OwnerMovementComponent = ComponentOwner->FindComponentByClass<UMovementComponent>();
		UCharacterMovementComponent* OwnerCharMovementComponent = Cast<UCharacterMovementComponent>(OwnerMovementComponent);
		if (OwnerMovementComponent && OwnerMovementComponent->UpdatedComponent == PrimitiveComponent)
		{
			if (OwnerCharMovementComponent && OwnerCharMovementComponent->MovementMode == EMovementMode::MOVE_Flying)
			{
				continue;
			}
			OwnerMovementComponent->AddRadialForce(Origin, Radius, EffectiveForceStrength, Falloff);
		}

		APawn* ComponentOwnerPawn = Cast<APawn>(ComponentOwner);
		if (DealsDamageToPawns && ComponentOwnerPawn)
		{
			const FVector CompOrigin = GetComponentLocation();
			const FVector PawnPosition = ComponentOwnerPawn->GetActorLocation();

			const float PawnDistance = FVector::Distance(ComponentOwnerPawn->GetActorLocation(), GetComponentLocation());
			const float EffectiveDamageRadius = Radius * 0.25f;
			const float Multiplier = FMath::Clamp(((PawnDistance - EffectiveDamageRadius) / EffectiveDamageRadius) * -1, 0.0f, 1.0f);
			const float Damage = 10.0f * Multiplier * (UpdateTime + 10.0f);

			FDamageEvent DamageEvent;
			ComponentOwnerPawn->TakeDamage(Damage, DamageEvent, nullptr, GetOwner());
		}
	}
}

bool UWindSuckerComponent::HasLineOfSightToActor(AActor* InActor) const
{
	if (InActor == nullptr)
	{
		return false;
	}

	const FVector InActorLocation = InActor->GetActorLocation();
	
	FHitResult HitResult;
	const FVector Start = GetComponentLocation();
	const FVector TowardComponentVector = (InActorLocation - Start).GetSafeNormal();
	const float DistanceFromComponent = FVector::Distance(Start, InActorLocation);
	const FVector End = Start + (TowardComponentVector * DistanceFromComponent);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this->GetOwner());

	if (!GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, Params))
	{
		return false;
	}

	if (HitResult.GetActor() == nullptr)
	{
		return false;
	}

	return HitResult.GetActor() == InActor;
}

void UWindSuckerComponent::SetRadius(float InRadius)
{
	Radius = InRadius;
}

void UWindSuckerComponent::SetForceStrength(float InForceStrength)
{
	ForceStrength = InForceStrength;
}

float UWindSuckerComponent::GetRadius() const
{
	return Radius;
}

float UWindSuckerComponent::GetForceStrength() const
{
	return ForceStrength;
}

void UWindSuckerComponent::SetDamagesPawn(bool InShouldDamagePawns)
{
	DealsDamageToPawns = InShouldDamagePawns;
}

bool UWindSuckerComponent::GetDamagesPawn() const
{
	return DealsDamageToPawns;
}

void UWindSuckerComponent::AddCollisionChannelToAffect(ECollisionChannel CollisionChannel)
{
	EObjectTypeQuery ObjectType = UEngineTypes::ConvertToObjectType(CollisionChannel);
	if (ObjectType != ObjectTypeQuery_MAX)
	{
		AddObjectTypeToAffect(ObjectType);
	}
}

void UWindSuckerComponent::AddObjectTypeToAffect(TEnumAsByte<EObjectTypeQuery> ObjectType)
{
	ObjectTypesToAffect.AddUnique(ObjectType);
	UpdateCollisionObjectQueryParams();
}

void UWindSuckerComponent::RemoveObjectTypeToAffect(TEnumAsByte<EObjectTypeQuery> ObjectType)
{
	ObjectTypesToAffect.Remove(ObjectType);
	UpdateCollisionObjectQueryParams();
}

void UWindSuckerComponent::UpdateCollisionObjectQueryParams()
{
	CollisionObjectQueryParams = FCollisionObjectQueryParams(ObjectTypesToAffect);
}