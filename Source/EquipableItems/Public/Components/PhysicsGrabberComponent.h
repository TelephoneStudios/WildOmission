// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "PhysicsGrabberComponent.generated.h"

UCLASS(ClassGroup = (Cutstom), meta = (BlueprintSpawnableComponent))
class EQUIPABLEITEMS_API UPhysicsGrabberComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPhysicsGrabberComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void ToggleGrab();

private:
	UPROPERTY(EditAnywhere)
	float MaxGrabDistance = 400.0f;

	UPROPERTY(EditAnywhere)
	float HoldDistance = 100.0f;

	UPROPERTY(EditAnywhere)
	float GrabRadius = 100.0f;

	class UPhysicsHandleComponent* GetPhysicsHandle() const;

	bool GetGrabbableInReach(FHitResult& OutHitResult) const;

	UFUNCTION(BlueprintCallable)
	void Grab();

	UFUNCTION(BlueprintCallable)
	void Release();

};