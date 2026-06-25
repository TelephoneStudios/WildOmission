// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "PhysicsGrabber.generated.h"

UCLASS(ClassGroup = (Cutstom), meta = (BlueprintSpawnableComponent))
class INVENTORY_API UPhysicsGrabber : public USceneComponent
{
	GENERATED_BODY()

public:
	UPhysicsGrabber();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void Grab();

	UFUNCTION(BlueprintCallable)
	void Release();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	float MaxGrabDistance = 400.0f;

	UPROPERTY(EditAnywhere)
	float HoldDistance = 200.0f;

	UPROPERTY(EditAnywhere)
	float GrabRadius = 100.0f;

	class UPhysicsHandleComponent* GetPhysicsHandle() const;

	bool GetGrabbableInReach(FHitResult& OutHitResult) const;

};