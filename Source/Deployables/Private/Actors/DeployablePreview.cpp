// Copyright Telephone Studios. All Rights Reserved.


#include "DeployablePreview.h"
#include "Deployables/Deployable.h"
#include "Log.h"

static UMaterialInstance* PreviewMaterial = nullptr;

ADeployablePreview::ADeployablePreview()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	SetMobility(EComponentMobility::Movable);
	GetStaticMeshComponent()->SetCollisionProfileName(TEXT("NoCollision"));
	CollisionCheckMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollisionMeshComponent"));
	CollisionCheckMeshComponent->SetupAttachment(GetStaticMeshComponent());
	CollisionCheckMeshComponent->SetHiddenInGame(true);
	CollisionCheckMeshComponent->SetCollisionProfileName(TEXT("OverlapAll"));
	CollisionCheckMeshComponent->SetGenerateOverlapEvents(true);
	CollisionCheckMeshComponent->SetRelativeScale3D(FVector(0.9f));

	InvalidOverlapCount = 0;
	GroundOverlapCount = 0;

	PreviewingDeployable = nullptr;
	PreviewMaterial = nullptr;

	static ConstructorHelpers::FObjectFinder<UMaterialInstance> PreviewMaterialInstanceBlueprint(TEXT("/Game/Deployables/Art/M_DeployablePreview_Inst"));
	if (PreviewMaterialInstanceBlueprint.Succeeded())
	{
		PreviewMaterial = PreviewMaterialInstanceBlueprint.Object;
	}
}

void ADeployablePreview::Setup(ADeployable* DeployableToPreview)
{
	if (DeployableToPreview == nullptr)
	{
		UE_LOG(LogDeployables, Warning, TEXT("Cannot create preview from a null deployable."));
		return;
	}

	PreviewingDeployable = DeployableToPreview;

	GetStaticMeshComponent()->SetStaticMesh(PreviewingDeployable->GetMesh());
	GetStaticMeshComponent()->SetRelativeTransform(PreviewingDeployable->GetMeshTransform());
	
	// Set all materials on the mesh to use the preview material
	for (int32 i = 0; i < GetStaticMeshComponent()->GetNumMaterials(); i++)
	{
		GetStaticMeshComponent()->SetMaterial(i, PreviewMaterial);
	}

	CollisionCheckMeshComponent->SetStaticMesh(PreviewingDeployable->GetMesh());

	CollisionCheckMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &ADeployablePreview::OnMeshBeginOverlap);
	CollisionCheckMeshComponent->OnComponentEndOverlap.AddDynamic(this, &ADeployablePreview::OnMeshEndOverlap);
}

void ADeployablePreview::Setup(UStaticMesh* PreviewMesh)
{
	PreviewingDeployable = nullptr;

	GetStaticMeshComponent()->SetStaticMesh(PreviewMesh);
	GetStaticMeshComponent()->SetRelativeTransform(FTransform());

	// Set all materials on the mesh to use the preview material
	for (int32 i = 0; i < GetStaticMeshComponent()->GetNumMaterials(); i++)
	{
		GetStaticMeshComponent()->SetMaterial(i, PreviewMaterial);
	}

	CollisionCheckMeshComponent->SetStaticMesh(PreviewMesh);

	CollisionCheckMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &ADeployablePreview::OnMeshBeginOverlap);
	CollisionCheckMeshComponent->OnComponentEndOverlap.AddDynamic(this, &ADeployablePreview::OnMeshEndOverlap);
}

void ADeployablePreview::Update(bool IsSpawnValid)
{
	GetStaticMeshComponent()->SetScalarParameterValueOnMaterials(TEXT("Valid"), IsSpawnValid);
	UE_LOG(LogTemp, Warning, TEXT("%i grounded"), IsGrounded());
}

bool ADeployablePreview::IsOverlappingInvalidObject() const
{
	return InvalidOverlapCount > 0;
}

bool ADeployablePreview::MustBeGrounded() const
{
	if (PreviewingDeployable == nullptr)
	{
		return false;
	}

	return PreviewingDeployable->CanSpawnOnGround() && !PreviewingDeployable->CanSpawnOnFloor();
}

bool ADeployablePreview::IsGrounded() const
{
	return GroundOverlapCount > 0;
}

void ADeployablePreview::OnMeshBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	if (OtherActor == nullptr
		|| OtherComponent == nullptr)
	{
		return;
	}

	if (OtherComponent->ComponentHasTag(TEXT("Ground")))
	{
		GroundOverlapCount++;
	}

	if (OtherComponent->ComponentHasTag(TEXT("Water")) 
		|| PreviewingDeployable == nullptr
		|| (OtherComponent->ComponentHasTag(TEXT("Ground")) && PreviewingDeployable->CanSpawnOnGround()) 
		|| (OtherActor->ActorHasTag(TEXT("Wall")) && PreviewingDeployable->CanSpawnOnWall()) 
		|| (OtherActor->ActorHasTag(TEXT("Floor")) && PreviewingDeployable->CanSpawnOnFloor()) 
		|| (OtherActor->ActorHasTag(TEXT("Floor")) && PreviewingDeployable->CanSpawnOnCeiling()) 
		|| (PreviewingDeployable->CanSpawnOnBuildAnchor() != None && (OtherActor->ActorHasTag(TEXT("BuildingPart")) || OtherComponent->ComponentHasTag(TEXT("Ground")))))
	{
		return;
	}

	InvalidOverlapCount++;
}

void ADeployablePreview::OnMeshEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	if (OtherActor == nullptr
		|| OtherComponent == nullptr)
	{
		return;
	}
	
	if (OtherComponent->ComponentHasTag(TEXT("Ground")))
	{
		GroundOverlapCount--;
	}

	if (OtherComponent->ComponentHasTag(TEXT("Water"))
		|| PreviewingDeployable == nullptr
		|| (OtherComponent->ComponentHasTag(TEXT("Ground")) && PreviewingDeployable->CanSpawnOnGround())
		|| (OtherActor->ActorHasTag(TEXT("Wall")) && PreviewingDeployable->CanSpawnOnWall())
		|| (OtherActor->ActorHasTag(TEXT("Floor")) && PreviewingDeployable->CanSpawnOnFloor())
		|| (OtherActor->ActorHasTag(TEXT("Floor")) && PreviewingDeployable->CanSpawnOnCeiling())
		|| (PreviewingDeployable->CanSpawnOnBuildAnchor() != None && (OtherActor->ActorHasTag(TEXT("BuildingPart")) || OtherComponent->ComponentHasTag(TEXT("Ground")))))
	{
		return;
	}

	InvalidOverlapCount--;
}