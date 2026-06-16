// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/CharacterSettingsInterface.h"
#include "WildOmissionCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDeathSignature, const FVector&, DeathLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKilledAnotherPlayerSignature, APlayerController*, KilledPlayer);

UCLASS()
class WILDOMISSIONCORE_API AWildOmissionCharacter : public ACharacter, public ICharacterSettingsInterface
{
	GENERATED_BODY()

public:
	AWildOmissionCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	virtual void Tick(float DeltaTime) override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	virtual void BeginDestroy() override;

	virtual void Jump() override;
	virtual void Landed(const FHitResult& HitResult) override;

	UFUNCTION(BlueprintCallable)
	void HandleFly();

	// Begin ICharacterSettingsInterface Implementation
	virtual void ApplyInputSettings() override;
	virtual void ApplyGameplaySettings() override;
	virtual void ApplyPostProcessingSettings() override;
	// End ICharacterSettingsInterface Implementation

	UFUNCTION()
	void HandleDeath();

	UPROPERTY(BlueprintAssignable)
	FOnPlayerDeathSignature OnPlayerDeath;
	UPROPERTY(BlueprintAssignable)
	FOnKilledAnotherPlayerSignature OnKilledAnotherPlayerSignature;
	
	UFUNCTION()
	void SetGodMode(bool GodMode);

	UFUNCTION(BlueprintCallable)
	USkeletalMeshComponent* GetArmsMesh() const;

	UFUNCTION()
	float GetHeadPitch() const;

	UFUNCTION()
	FRotator GetReplicatedControlRotation() const;

	UFUNCTION()
	bool IsSprinting() const;

	UFUNCTION()
	bool IsAiming() const;

	UFUNCTION()
	bool IsUnderwater() const;

	UFUNCTION()
	bool IsMovementButtonHeld() const;

	UFUNCTION()
	UCameraComponent* GetFirstPersonCameraComponent();

	UFUNCTION(BlueprintCallable)
	class UPlayerHUDWidget* GetHUDWidget() const;

	UFUNCTION(BlueprintCallable)
	class UVitalsComponent* GetVitalsComponent() const;

	UFUNCTION(BlueprintCallable)
	class UInventoryManipulatorComponent* GetInventoryManipulatorComponent() const;

	UFUNCTION(BlueprintCallable)
	class UPlayerInventoryComponent* GetInventoryComponent() const;

	UFUNCTION(BlueprintCallable)
	class UCraftingComponent* GetCraftingComponent() const;

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* FirstPersonArmsMeshComponent;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* FirstPersonCameraComponent;
	
	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* FirstPersonSpringArmComponent;

	UPROPERTY(VisibleAnywhere)
	class UChunkInvokerComponent* ChunkInvokerComponent;

	UPROPERTY(VisibleAnywhere)
	class UVitalsComponent* VitalsComponent;

	UPROPERTY(VisibleAnywhere)
	class UInteractionComponent* InteractionComponent;

	UPROPERTY(VisibleAnywhere)
	class UPlayerInventoryComponent* InventoryComponent;
	
	UPROPERTY(VisibleAnywhere)
	class UInventoryManipulatorComponent* InventoryManipulatorComponent;

	UPROPERTY(VisibleAnywhere)
	class UEquipComponent* EquipComponent;
	
	UPROPERTY(VisibleAnywhere)
	class UPlayerAimComponent* AimComponent;

	UPROPERTY(VisibleAnywhere)
	class UCraftingComponent* CraftingComponent;

	UPROPERTY(VisibleAnywhere)
	class UBuilderComponent* BuilderComponent;

	UPROPERTY(VisibleAnywhere)
	class UNameTagComponent* NameTag;

	UPROPERTY(VisibleAnywhere)
	class USpecialEffectsManagerComponent* SpecialEffectsManagerComponent;

	UPROPERTY(VisibleAnywhere)
	class ULockModifierComponent* LockModifierComponent;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCameraShakeBase> WalkCameraShake;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCameraShakeBase> RunCameraShake;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCameraShakeBase> JumpCameraShake;

	UPROPERTY()
	class UPlayerHUDWidget* PlayerHUDWidget;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UPlayerHUDWidget> PlayerHUDWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class ALootableRagdoll> RagdollClass;

	UPROPERTY(EditDefaultsOnly)
	USoundBase* FallCrunchSound;

	UPROPERTY(EditDefaultsOnly)
	USoundBase* SpookySoundCue;
	
	float TimeToNextSpookySound;
	float SpookyCounter;

	UPROPERTY(Replicated)
	FRotator ReplicatedControlRotation;

	UPROPERTY()
	bool bSprinting;

	UPROPERTY()
	bool bSprintButtonHeld;

	UPROPERTY()
	bool bGrounded;

	UPROPERTY()
	bool bUnderwater;

	void HandleUnderwater();

	UFUNCTION()
	void PlaySpookySound();

	//*****************************
	// Input
	//*****************************
	UPROPERTY()
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY()
	class UInputAction* MoveForwardAction;

	UPROPERTY()
	class UInputAction* MoveBackwardAction;

	UPROPERTY()
	class UInputAction* MoveLeftAction;

	UPROPERTY()
	class UInputAction* MoveRightAction;

	UPROPERTY()
	class UInputAction* LookAction;
	
	UPROPERTY()
	class UInputAction* SprintAction;

	UPROPERTY()
	class UInputAction* CrouchAction;

	UPROPERTY()
	class UInputAction* JumpAction;

	UPROPERTY()
	class UInputAction* FlyAction;
		
	UPROPERTY()
	class UInputAction* PrimaryAction;
	
	UPROPERTY()
	class UInputAction* SecondaryAction;

	UPROPERTY()
	class UInputAction* InteractAction;

	UPROPERTY()
	class UInputAction* ReloadAction;
	
	UPROPERTY()
	class UInputAction* ToolbarSelectionIncrementAction;
	
	UPROPERTY()
	class UInputAction* ToolbarSelectionDecrementAction;

	UPROPERTY()
	class UInputAction* ToolbarSelection1Action;
	
	UPROPERTY()
	class UInputAction* ToolbarSelection2Action;
	
	UPROPERTY()
	class UInputAction* ToolbarSelection3Action;
	
	UPROPERTY()
	class UInputAction* ToolbarSelection4Action;
	
	UPROPERTY()
	class UInputAction* ToolbarSelection5Action;
	
	UPROPERTY()
	class UInputAction* ToolbarSelection6Action;
	
	UPROPERTY()
	class UInputAction* ToggleInventoryMenuAction;

	UPROPERTY()
	class UInputAction* ToggleCraftingMenuAction;

	UPROPERTY()
	class UInputAction* ToggleChatAction;

	bool LookUpInverted;

	UFUNCTION()
	void StartMovementViewBobbing();
	
	UFUNCTION()
	void UpdateMovementViewBobbingState();

	UFUNCTION()
	void StopMovementViewBobbing();


	UFUNCTION()
	void MoveForward();

	UFUNCTION()
	void MoveBackward();

	UFUNCTION()
	void MoveLeft();

	UFUNCTION()
	void MoveRight();

	UPROPERTY()
	bool bMoveForwardHeld;
	UPROPERTY()
	bool bMoveBackwardHeld;
	UPROPERTY()
	bool bMoveLeftHeld;
	UPROPERTY()
	bool bMoveRightHeld;

	UFUNCTION()
	void OnMoveForwardPressed();
	UFUNCTION()
	void OnMoveForwardReleased();

	UFUNCTION()
	void OnMoveBackwardPressed();
	UFUNCTION()
	void OnMoveBackwardReleased();

	UFUNCTION()
	void OnMoveLeftPressed();
	UFUNCTION()
	void OnMoveLeftReleased();

	UFUNCTION()
	void OnMoveRightPressed();
	UFUNCTION()
	void OnMoveRightReleased();

	UFUNCTION()
	void Look(const struct FInputActionValue& Value);

	UFUNCTION()
	void OnSprintButtonPressed();
	UFUNCTION()
	void OnSprintButtonReleased();

	UFUNCTION()
	void StartSprinting();
	UFUNCTION()
	void StopSprinting();

	UFUNCTION()
	void StartCrouching();
	UFUNCTION()
	void StopCrouching();

	UFUNCTION()
	void OnJumpHeld();
	UFUNCTION()
	void OnCrouchHeld();

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_MovementSpeed)
	float DesiredMovementSpeed;

	UPROPERTY(EditDefaultsOnly)
	float WalkMovementSpeed;

	UPROPERTY(EditDefaultsOnly)
	float SprintMovementSpeed;

	UPROPERTY(EditDefaultsOnly)
	float AimMovementSpeed;
	
	UFUNCTION()
	void OnRep_MovementSpeed();

	UFUNCTION()
	void RefreshDesiredMovementSpeed();

	UFUNCTION(Server, Reliable)
	void Server_Sprint(bool bShouldSprint);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_HandleFly();

	UFUNCTION(NetMulticast, Reliable)
	void Multi_PlayFallCrunchSound();

	UFUNCTION()
	void PrimaryPressed();
	UFUNCTION()
	void PrimaryReleased();

	UFUNCTION()
	void SecondaryPressed();
	UFUNCTION()
	void SecondaryReleased();

	UFUNCTION()
	void ReloadPressed();

	UFUNCTION()
	void ToggleInventoryMenu();
	
	UFUNCTION()
	void ToggleCraftingMenu();

	UFUNCTION()
	void ToggleChat();

	UFUNCTION()
	void ToolbarSelectionIncrement();
	
	UFUNCTION()
	void ToolbarSelectionDecrement();

	UFUNCTION()
	void SelectToolbarSlot1();
	UFUNCTION()
	void SelectToolbarSlot2();
	UFUNCTION()
	void SelectToolbarSlot3();
	UFUNCTION()
	void SelectToolbarSlot4();
	UFUNCTION()
	void SelectToolbarSlot5();
	UFUNCTION()
	void SelectToolbarSlot6();

	//********************************	
	// Setup Member functions
	//********************************

	void SetupEnhancedInputSubsystem();
	
	void SetupMesh();
	
	void SetupGameMode();

	void SetupPlayerHUD();

	void SetupLocalComponents();

};
