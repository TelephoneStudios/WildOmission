// Copyright Telephone Studios. All Rights Reserved.


#include "Actors/Tornado.h"
#include "Components/WindSuckerComponent.h"
#include "Components/ChunkInvokerComponent.h"
#include "Interfaces/DamagedByWind.h"
#include "Interfaces/InteractsWithTornado.h"
#include "Engine/OverlapResult.h"
#include "Log.h"

// Sets default values
ATornado::ATornado()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	bAlwaysRelevant = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RootComponent = MeshComponent;

	ChunkInvokerComponent = CreateDefaultSubobject<UChunkInvokerComponent>(TEXT("ChunkInvokerComponent"));
	ChunkInvokerComponent->SetupAttachment(RootComponent);
	ChunkInvokerComponent->SetRenderDistance(8);

	SuctionAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("SuctionAnchor"));
	SuctionAnchor->SetupAttachment(MeshComponent);
	SuctionAnchor->SetRelativeLocation(FVector(0.0f, 0.0f, 3000.0f));

	CloseSuctionComponent1 = CreateDefaultSubobject<UWindSuckerComponent>(TEXT("CloseSuctionComponent1"));
	CloseSuctionComponent1->SetupAttachment(SuctionAnchor);
	CloseSuctionComponent1->SetRelativeLocation(FVector(2000.0f, 0.0f, 0.0f));
	
	CloseSuctionComponent2 = CreateDefaultSubobject<UWindSuckerComponent>(TEXT("CloseSuctionComponent2"));
	CloseSuctionComponent2->SetupAttachment(SuctionAnchor);
	CloseSuctionComponent2->SetRelativeLocation(FVector(-2000.0f, 0.0f, 0.0f));

	CloseSuctionComponent3 = CreateDefaultSubobject<UWindSuckerComponent>(TEXT("CloseSuctionComponent3"));
	CloseSuctionComponent3->SetupAttachment(SuctionAnchor);
	CloseSuctionComponent3->SetRelativeLocation(FVector(0.0f, 2000.0f, 0.0f));

	CloseSuctionComponent4 = CreateDefaultSubobject<UWindSuckerComponent>(TEXT("CloseSuctionComponent4"));
	CloseSuctionComponent4->SetupAttachment(SuctionAnchor);
	CloseSuctionComponent4->SetRelativeLocation(FVector(0.0f, -2000.0f, 0.0f));

	FarSuctionComponent = CreateDefaultSubobject<UWindSuckerComponent>(TEXT("FarSuctionComponent"));
	FarSuctionComponent->SetRadius(30000.0f);
	FarSuctionComponent->SetForceStrength(-99999.0f);
	FarSuctionComponent->SetupAttachment(SuctionAnchor);
	FarSuctionComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 3000.0f));
	FarSuctionComponent->SetDamagesPawn(true);

	RotationSpeed = 30.0f;
	MovementSpeed = 1000.0f;
	TargetLocation = FVector::ZeroVector;
	BoundsRadius = 0.0f;
	RemainingLifetime = 0.0f;
	TotalLifetime = 0.0f;
	Severity = 0.0f;
}

// Called when the game starts or when spawned
void ATornado::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	MeshComponent->OnComponentBeginOverlap.AddDynamic(this, &ATornado::OnActorOverlapVortex);
}

void ATornado::Setup(bool SpawnedFromCommand)
{
	this->SetActorRelativeLocation(GetRandomLocationInBounds());
	
	TargetLocation = GetRandomLocationInBounds();

	if (SpawnedFromCommand)
	{
		this->SetActorRelativeLocation(FVector(-BoundsRadius * 0.25f, 0.0f, 0.0f));
		TargetLocation = FVector(BoundsRadius * 0.25f, 0.0f, 0.0f);
	}
	

	// Assign random lifetime (seconds)
	TotalLifetime = FMath::RandRange(120.0f, 300.0f);
	// Assign Severity F1 - F5
	Severity = FMath::RandRange(int32(1), int32(5));
	OnUpdateSeverity();
	RemainingLifetime = TotalLifetime;
}

void ATornado::SetBoundsRadius(float InBoundsRadius)
{
	BoundsRadius = InBoundsRadius;
}

float ATornado::GetBoundsRadius() const
{
	return BoundsRadius;
}

FTornadoData ATornado::GetTornadoData() const
{
	FTornadoData NewData;
	NewData.Transform = MeshComponent->GetRelativeTransform();
	NewData.MovementSpeed = MovementSpeed;
	NewData.TotalLifetime = TotalLifetime;
	NewData.RemainingLifetime = RemainingLifetime;
	NewData.RotationSpeed = RotationSpeed;
	NewData.TargetLocation = TargetLocation;
	NewData.Severity = Severity;

	return NewData;
}

void ATornado::LoadTornadoData(const FTornadoData& InTornadoData)
{
	MeshComponent->SetRelativeTransform(InTornadoData.Transform);
	MovementSpeed = InTornadoData.MovementSpeed;
	TotalLifetime = InTornadoData.TotalLifetime;
	RemainingLifetime = InTornadoData.RemainingLifetime;
	RotationSpeed = InTornadoData.RotationSpeed;
	TargetLocation = InTornadoData.TargetLocation;
	Severity = InTornadoData.Severity;
	OnUpdateSeverity();
}

// Called every frame
void ATornado::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateRotation();

	if (!HasAuthority())
	{
		return;
	}

	UpdateMovement();
	UpdateDamage();
	RemainingLifetime -= DeltaTime;

	if (RemainingLifetime < KINDA_SMALL_NUMBER)
	{
		this->Destroy();
	}
}

void ATornado::OnActorOverlapVortex(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IInteractsWithTornado* InteractsWithActor = Cast<IInteractsWithTornado>(OtherActor))
	{
		InteractsWithActor->OnContactWithTornado();
	}
}

void ATornado::UpdateMovement()
{
	const UWorld* World = GetWorld();
	const AActor* OwnerActor = GetOwner();
	if (World == nullptr || OwnerActor == nullptr || MeshComponent == nullptr)
	{
		return;
	}

	const FVector CurrentLocation(MeshComponent->GetRelativeLocation().X, MeshComponent->GetRelativeLocation().Y, 0.0f);
	const FVector VectorTowardTarget((TargetLocation - CurrentLocation).GetSafeNormal());
	const float GroundLevelZ = -OwnerActor->GetActorLocation().Z;
	const float DistanceFromTarget = FVector::Distance(TargetLocation, CurrentLocation);
	
	if (DistanceFromTarget < 100.0f)
	{
		UE_LOG(LogWeather, Verbose, TEXT("Getting New Target Location For Tornado."));
		TargetLocation = GetRandomLocationInBounds();
	}

	const float TotalTimeAlive = TotalLifetime - RemainingLifetime;
	FVector NewLocation = CurrentLocation + (VectorTowardTarget * MovementSpeed * World->GetDeltaSeconds());

	float ZAxis = GroundLevelZ;
	if (TotalTimeAlive < 5.0f)
	{
		ZAxis = FMath::Lerp(0.0f, GroundLevelZ, TotalTimeAlive / 5.0f);
	}
	else if (RemainingLifetime < 5.0f)
	{
		ZAxis = FMath::Lerp(GroundLevelZ, 0.0f, (5.0f / RemainingLifetime) - 1.0f);
	}

	NewLocation.Z = ZAxis;

	MeshComponent->SetRelativeLocation(NewLocation);
}

void ATornado::UpdateDamage()
{
	const UWorld* World = GetWorld();
	if (World == nullptr || FarSuctionComponent == nullptr)
	{
		return;
	}

	TArray<FOverlapResult> Overlaps;
	FVector WindOrigin = FarSuctionComponent->GetComponentLocation();
	float WindRadius = FarSuctionComponent->GetRadius() * 0.5f;
	FCollisionObjectQueryParams ObjectParams(ECollisionChannel::ECC_WorldDynamic);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	World->OverlapMultiByObjectType(Overlaps, WindOrigin, FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(WindRadius), Params);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		IDamagedByWind* DamagedByWindActor = Cast<IDamagedByWind>(Overlap.GetActor());
		if (DamagedByWindActor == nullptr || !HasLineOfSightTo(Overlap.GetActor()))
		{
			continue;
		}

		const float DistanceFromOrigin = FVector::Distance(Overlap.GetActor()->GetActorLocation(), WindOrigin);
		const float DamageMultiplier = FMath::Clamp(((DistanceFromOrigin - WindRadius) / WindRadius) * -1, 0.0f, 1.0f + Severity);
		DamagedByWindActor->ApplyWindDamage(this, DamageMultiplier);
	}
}

void ATornado::UpdateRotation()
{
	const UWorld* World = GetWorld();
	if (World == nullptr || MeshComponent == nullptr || SuctionAnchor == nullptr)
	{
		return;
	}

	float NewYaw = MeshComponent->GetRelativeRotation().Yaw - (RotationSpeed * World->GetDeltaSeconds());
	float NewSuctionYaw = SuctionAnchor->GetRelativeRotation().Yaw - ((RotationSpeed * 4) * World->GetDeltaSeconds());

	if (NewYaw < -360.0f)
	{
		NewYaw = 0.0f;
	}

	if (NewSuctionYaw < -360.0f)
	{
		NewSuctionYaw = 0.0f;
	}

	MeshComponent->SetRelativeRotation(FRotator(0.0f, NewYaw, 0.0f));
	SuctionAnchor->SetRelativeRotation(FRotator(0.0f, NewSuctionYaw, 0.0f));
}

FVector ATornado::GetRandomLocationInBounds() const
{
	const float HalfRadius = BoundsRadius * 0.5f;
	return FVector(FMath::RandRange(-HalfRadius, HalfRadius), FMath::RandRange(-HalfRadius, HalfRadius), 0.0f);
}

void ATornado::OnUpdateSeverity()
{
	UE_LOG(LogTemp, Display, TEXT("Tornado '%s' has a Severity of %f"), *this->GetActorNameOrLabel(), Severity);
	this->SetActorRelativeScale3D(FVector(Severity, Severity, 1.0f));
}

bool ATornado::HasLineOfSightTo(AActor* InActor) const
{
	const UWorld* World = GetWorld();
	if (World == nullptr || InActor == nullptr)
	{
		return false;
	}

	const FVector InActorLocation = InActor->GetActorLocation();
	const FVector CurrentLocation = this->GetActorLocation();

	FHitResult HitResult;
	const FVector TraceStart = FVector(CurrentLocation.X, CurrentLocation.Y, InActorLocation.Z);
	const FVector TowardComponentVector = (InActorLocation - TraceStart).GetSafeNormal();
	const float DistanceFromComponent = FVector::Distance(TraceStart, InActorLocation);
	const FVector TraceEnd = TraceStart + (TowardComponentVector * DistanceFromComponent);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	if (!World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, Params))
	{
		return false;
	}

	AActor* HitActor = HitResult.GetActor();
	return HitActor == InActor;
}