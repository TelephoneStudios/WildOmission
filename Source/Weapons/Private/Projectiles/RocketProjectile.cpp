// Copyright Telephone Studios. All Rights Reserved.


#include "Projectiles/RocketProjectile.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/DamageEvents.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"

ARocketProjectile::ARocketProjectile()
{
    ExplosionSound = nullptr;
    ExplosionSystem = nullptr;
}

// On hit is only called on the server, effects like sound and particles are triggered through RPC calls
// Explosion damage is handled here
void ARocketProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // Just a quick check to make sure we arent getting used incorrectly
    if (!HasAuthority())
    {
        Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
        return;
    }

    // we will do an overlap test
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return;
    }

    const float ExplosionRadius = 400.0f;

    // whatever overlaps, we will do a damage event, and use our distance to calculate the damage
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Shape = FCollisionShape::MakeSphere(ExplosionRadius);
    World->OverlapMultiByChannel(Overlaps, this->GetActorLocation(), this->GetActorRotation().Quaternion(), ECollisionChannel::ECC_Visibility, Shape);
    if (Overlaps.IsEmpty())
    {
        return;
    }

    for (FOverlapResult Overlap : Overlaps)
    {
        AActor* OverlapedActor = Overlap.GetActor();
        if (OverlapedActor == nullptr)
        {
            continue;
        }

        if (Cast<APawn>(OverlapedActor))
        {
            PlayHitmarkerSound(false);
        }
        
        const float DistanceFromExplosionOrigin = FVector::Distance(OverlapedActor->GetActorLocation(), Hit.ImpactPoint);
        const float DamageMultiplier = (ExplosionRadius / DistanceFromExplosionOrigin) - 1.0f;
        const float DamageAmount = 120.0f * DamageMultiplier;

        FPointDamageEvent DamageEvent;
        OverlapedActor->TakeDamage(DamageAmount, DamageEvent, Cast<AController>(this->GetOwner()), this);
    }

    Multi_PlayExplosionEffects();

    Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}


void ARocketProjectile::Multi_PlayExplosionEffects_Implementation()
{
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, this->GetActorLocation());
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionSystem, this->GetActorLocation());
}