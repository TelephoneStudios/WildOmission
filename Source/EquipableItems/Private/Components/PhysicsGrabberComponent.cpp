// Copyright Telephone Studios. All Rights Reserved.


#include "Components/PhysicsGrabberComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

UPhysicsGrabberComponent::UPhysicsGrabberComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UPhysicsGrabberComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	UPhysicsHandleComponent* PhysHandle = GetPhysicsHandle();
	if (PhysHandle->GetGrabbedComponent() != nullptr)
	{
		FVector TargetLocation = GetComponentLocation() + GetForwardVector() * HoldDistance;
		PhysHandle->SetTargetLocationAndRotation(TargetLocation, GetComponentRotation());
	}
}

void UPhysicsGrabberComponent::ToggleGrab()
{
	UPhysicsHandleComponent* PhysHandle = GetPhysicsHandle();
	if (PhysHandle == nullptr)
	{
		return;
	}

	if (!PhysHandle->GetGrabbedComponent())
	{
		this->Grab();
	}
	else
	{
		this->Release();
	}
}

void UPhysicsGrabberComponent::Grab()
{
	UPhysicsHandleComponent* PhysHandle = GetPhysicsHandle();
	if (PhysHandle == nullptr)
	{
		return;
	}

	FHitResult HitResult;
	if (GetGrabbableInReach(HitResult))
	{
		UPrimitiveComponent* HitComponent = HitResult.GetComponent();
		//HitComponent->SetSimulatePhysics(true);
		//HitComponent->GetOwner()->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		HitComponent->WakeAllRigidBodies();
		if (HitResult.GetActor())
		{
			HitResult.GetActor()->Tags.Add("Grabbed");
		}
		PhysHandle->GrabComponentAtLocationWithRotation(HitComponent, NAME_None, HitResult.ImpactPoint, GetComponentRotation());
	}
}

void UPhysicsGrabberComponent::Release()
{
	UPhysicsHandleComponent* PhysHandle = GetPhysicsHandle();
	if (PhysHandle == nullptr)
	{
		return;
	}
	UPrimitiveComponent* GrabbedComponent = PhysHandle->GetGrabbedComponent();
	if (GrabbedComponent == nullptr)
	{
		return;
	}

	GrabbedComponent->WakeAllRigidBodies();
	GrabbedComponent->GetOwner()->Tags.Remove("Grabbed");
	PhysHandle->ReleaseComponent();
}

UPhysicsHandleComponent* UPhysicsGrabberComponent::GetPhysicsHandle() const
{
	UPhysicsHandleComponent* PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (PhysicsHandle == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Physics Handle nullptr on actor: %s"), *GetOwner()->GetActorNameOrLabel());
	}

	return PhysicsHandle;
}

bool UPhysicsGrabberComponent::GetGrabbableInReach(FHitResult& OutHitResult) const
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;

	}
	FVector Start = GetComponentLocation();
	FVector End = Start + GetForwardVector() * MaxGrabDistance;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(GrabRadius);

	return World->SweepSingleByChannel(
		OutHitResult,
		Start, End,
		FQuat::Identity,
		ECC_GameTraceChannel11,
		Sphere
	);
}