// Copyright Telephone Studios. All Rights Reserved.


#include "Components/PhysicsGrabberComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

UPhysicsGrabberComponent::UPhysicsGrabberComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UPhysicsGrabberComponent::BeginPlay()
{
	Super::BeginPlay();
}

// TODO lots of possible crashes here
void UPhysicsGrabberComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (GetPhysicsHandle()->GetGrabbedComponent() != nullptr)
	{
		FVector TargetLocation = GetComponentLocation() + GetForwardVector() * HoldDistance;
		GetPhysicsHandle()->SetTargetLocationAndRotation(TargetLocation, GetComponentRotation());
	}
}

void UPhysicsGrabberComponent::ToggleGrab()
{
	UE_LOG(LogTemp, Warning, TEXT("ToggleGrab"));
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
	FHitResult HitResult;
	if (GetGrabbableInReach(HitResult) == true)
	{
		UE_LOG(LogTemp, Warning, TEXT("Got Grabbable"));
		UPrimitiveComponent* HitComponent = HitResult.GetComponent();
		//HitComponent->SetSimulatePhysics(true);
		//HitComponent->GetOwner()->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		HitComponent->WakeAllRigidBodies();
		HitResult.GetActor()->Tags.Add("Grabbed");
		GetPhysicsHandle()->GrabComponentAtLocationWithRotation(HitComponent, NAME_None, HitResult.ImpactPoint, GetComponentRotation());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Didn't Grabbable"));
	}
}

void UPhysicsGrabberComponent::Release()
{
	if (GetPhysicsHandle()->GetGrabbedComponent() != nullptr)
	{
		GetPhysicsHandle()->GetGrabbedComponent()->WakeAllRigidBodies();
		GetPhysicsHandle()->GetGrabbedComponent()->GetOwner()->Tags.Remove("Grabbed");
		GetPhysicsHandle()->ReleaseComponent();
	}
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
	FVector Start = GetComponentLocation();
	FVector End = Start + GetForwardVector() * MaxGrabDistance;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(GrabRadius);

	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 10.0f, 0, 10.0f);

	return GetWorld()->SweepSingleByChannel(
		OutHitResult,
		Start, End,
		FQuat::Identity,
		ECC_GameTraceChannel11,
		Sphere
	);
}