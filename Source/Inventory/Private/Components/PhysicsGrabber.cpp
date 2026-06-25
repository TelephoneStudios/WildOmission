// Copyright Telephone Studios. All Rights Reserved.


#include "Components/PhysicsGrabber.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

UPhysicsGrabber::UPhysicsGrabber()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UPhysicsGrabber::BeginPlay()
{
	Super::BeginPlay();
}

void UPhysicsGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (GetPhysicsHandle()->GetGrabbedComponent() != nullptr)
	{
		FVector TargetLocation = GetComponentLocation() + GetForwardVector() * HoldDistance;
		GetPhysicsHandle()->SetTargetLocationAndRotation(TargetLocation, GetComponentRotation());
	}
}

void UPhysicsGrabber::Grab()
{
	FHitResult HitResult;
	if (GetGrabbableInReach(HitResult) == true)
	{
		UPrimitiveComponent* HitComponent = HitResult.GetComponent();
		HitComponent->SetSimulatePhysics(true);
		HitComponent->GetOwner()->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		HitComponent->WakeAllRigidBodies();
		HitResult.GetActor()->Tags.Add("Grabbed");
		GetPhysicsHandle()->GrabComponentAtLocationWithRotation(HitComponent, NAME_None, HitResult.ImpactPoint, GetComponentRotation());
	}
}

void UPhysicsGrabber::Release()
{
	if (GetPhysicsHandle()->GetGrabbedComponent() != nullptr)
	{
		GetPhysicsHandle()->GetGrabbedComponent()->WakeAllRigidBodies();
		GetPhysicsHandle()->GetGrabbedComponent()->GetOwner()->Tags.Remove("Grabbed");
		GetPhysicsHandle()->ReleaseComponent();
	}
}

UPhysicsHandleComponent* UPhysicsGrabber::GetPhysicsHandle() const
{
	UPhysicsHandleComponent* PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (PhysicsHandle == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Physics Handle nullptr on actor: %s"), *GetOwner()->GetActorNameOrLabel());
	}

	return PhysicsHandle;
}

bool UPhysicsGrabber::GetGrabbableInReach(FHitResult& OutHitResult) const
{
	FVector Start = GetComponentLocation();
	FVector End = Start + GetForwardVector() * MaxGrabDistance;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(GrabRadius);

	return GetWorld()->SweepSingleByChannel(
		OutHitResult,
		Start, End,
		FQuat::Identity,
		ECC_GameTraceChannel2,
		Sphere
	);
}