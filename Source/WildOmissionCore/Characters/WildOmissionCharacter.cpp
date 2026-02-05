// Copyright Telephone Studios. All Rights Reserved.

// Character Core
#include "WildOmissionCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "HumanAnimInstance.h"
#include "WildOmissionCore/PlayerControllers/WildOmissionPlayerController.h"
#include "GameFramework/PlayerState.h"

// Wild Omission Components
#include "Components/ChunkInvokerComponent.h"
#include "Components/VitalsComponent.h"
#include "Components/InteractionComponent.h"
#include "Components/PlayerInventoryComponent.h"
#include "Components/InventoryManipulatorComponent.h"
#include "Components/EquipComponent.h"
#include "WildOmissionCore/Components/PlayerAimComponent.h"
#include "Components/CraftingComponent.h"
#include "Components/BuilderComponent.h"
#include "WildOmissionCore/Components/NameTagComponent.h"
#include "WildOmissionCore/Components/SpecialEffectsManagerComponent.h"
#include "Components/LockModifierComponent.h"

// Wild Omission Stuff
#include "WildOmissionCore/UI/Player/PlayerHUDWidget.h"
#include "GameChatManager.h"
#include "WildOmissionCore/GameModes/WildOmissionGameMode.h"
#include "WildOmissionGameUserSettings.h"
#include "Deployables/ItemContainerBase.h"
#include "Ragdolls/LootableRagdoll.h"

// Engine Stuff
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"

//********************************
// Setup/General Actor Functionality
//********************************
AWildOmissionCharacter::AWildOmissionCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PlayerHUDWidget = nullptr;

	RagdollClass = nullptr;

	USkeletalMeshComponent* ThirdPersonMeshComponent = GetMesh();
	ThirdPersonMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	ThirdPersonMeshComponent->SetRelativeScale3D(FVector(0.95f, 0.95f, 0.95f));

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(RootComponent);
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	FirstPersonSpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("FirstPersonSpringArmComponent"));
	FirstPersonSpringArmComponent->TargetArmLength = 0.0f;
	FirstPersonSpringArmComponent->bEnableCameraRotationLag = true;
	FirstPersonSpringArmComponent->CameraRotationLagSpeed = 25.0f;
	FirstPersonSpringArmComponent->SetupAttachment(FirstPersonCameraComponent);

	FirstPersonArmsMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonArmsComponent"));
	FirstPersonArmsMeshComponent->SetupAttachment(FirstPersonSpringArmComponent);
	FirstPersonArmsMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	FirstPersonArmsMeshComponent->SetRelativeLocation(FVector(-5.0f, 0.0f, -160.0f));
	FirstPersonArmsMeshComponent->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	FirstPersonArmsMeshComponent->SetRelativeScale3D(FVector(0.95f, 0.95f, 0.95f));
	FirstPersonArmsMeshComponent->SetVisibility(false);
	FirstPersonArmsMeshComponent->SetCastShadow(false);

	ChunkInvokerComponent = CreateDefaultSubobject<UChunkInvokerComponent>(TEXT("ChunkInvokerComponent"));
	ChunkInvokerComponent->SetupAttachment(RootComponent);
	ChunkInvokerComponent->SetRenderDistance(16);

	VitalsComponent = CreateDefaultSubobject<UVitalsComponent>(TEXT("VitalsComponent"));

	InventoryManipulatorComponent = CreateDefaultSubobject<UInventoryManipulatorComponent>(TEXT("InventoryManipulatorComponent"));

	InventoryComponent = CreateDefaultSubobject<UPlayerInventoryComponent>(TEXT("InventoryComponent"));

	EquipComponent = CreateDefaultSubobject<UEquipComponent>(TEXT("EquipComponent"));
	EquipComponent->SetupAttachment(FirstPersonCameraComponent);
	EquipComponent->Setup(FirstPersonArmsMeshComponent, ThirdPersonMeshComponent);

	AimComponent = CreateDefaultSubobject<UPlayerAimComponent>(TEXT("AimComponent"));

	CraftingComponent = CreateDefaultSubobject<UCraftingComponent>(TEXT("CraftingComponent"));

	BuilderComponent = CreateDefaultSubobject<UBuilderComponent>(TEXT("BuilderComponent"));

	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
	InteractionComponent->SetupAttachment(FirstPersonCameraComponent);

	NameTag = CreateDefaultSubobject<UNameTagComponent>(TEXT("NameTag"));
	NameTag->SetupAttachment(RootComponent);
	NameTag->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));

	SpecialEffectsManagerComponent = nullptr;
	
	LockModifierComponent = CreateDefaultSubobject<ULockModifierComponent>(TEXT("LockModifierComponent"));

	TimeToNextSpookySound = 300.0f;
	SpookyCounter = 0.0f;

	bSprinting = false;
	bSprintButtonHeld = false;
	bUnderwater = false;
	
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	CharacterMovementComponent->GravityScale = 1.5f;
	CharacterMovementComponent->MaxAcceleration = 10000.0f;
	CharacterMovementComponent->GroundFriction = 10.0f;
	CharacterMovementComponent->AirControl = 0.0f;
	CharacterMovementComponent->AirControlBoostMultiplier = 0.0f;
	CharacterMovementComponent->FallingLateralFriction = 0.3f;

	CharacterMovementComponent->JumpZVelocity = 550.0f;
	CharacterMovementComponent->MaxWalkSpeed = 300.0f;

	DesiredMovementSpeed = 300.0f;
	
	WalkMovementSpeed = 300.0f;
	SprintMovementSpeed = 600.0f;
	AimMovementSpeed = 100.0f;

	LookUpInverted = false;

	static ConstructorHelpers::FClassFinder<UCameraShakeBase> JumpCameraShakeBlueprint(TEXT("/Game/WildOmissionCore/Characters/Human/Effects/CS_Jump"));
	if (JumpCameraShakeBlueprint.Succeeded())
	{
		JumpCameraShake = JumpCameraShakeBlueprint.Class;
	}

	static ConstructorHelpers::FClassFinder<UPlayerHUDWidget> PlayerHUDWidgetBlueprintClass(TEXT("/Game/WildOmissionCore/UI/Player/WBP_PlayerHUD"));
	if (PlayerHUDWidgetBlueprintClass.Succeeded())
	{
		PlayerHUDWidgetClass = PlayerHUDWidgetBlueprintClass.Class;
	}

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> PlayerArmsMeshObject(TEXT("/Game/WildOmissionCore/Art/Characters/SK_HumanFirstPersonArms"));
	if (PlayerArmsMeshObject.Succeeded())
	{
		FirstPersonArmsMeshComponent->SetSkeletalMesh(PlayerArmsMeshObject.Object);
	}

	static ConstructorHelpers::FClassFinder<UHumanAnimInstance> PlayerArmsAnimBlueprintClass(TEXT("/Game/WildOmissionCore/Characters/Human/Animation/ABP_Human_FirstPerson"));
	if (PlayerArmsAnimBlueprintClass.Succeeded())
	{
		FirstPersonArmsMeshComponent->SetAnimClass(PlayerArmsAnimBlueprintClass.Class);
	}
	
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> PlayerThirdPersonMeshObject(TEXT("/Game/WildOmissionCore/Art/Characters/SK_Human"));
	if (PlayerThirdPersonMeshObject.Succeeded())
	{
		ThirdPersonMeshComponent->SetSkeletalMesh(PlayerThirdPersonMeshObject.Object);
	}

	static ConstructorHelpers::FClassFinder<UHumanAnimInstance> PlayerThirdPersonAnimBlueprintClass(TEXT("/Game/WildOmissionCore/Characters/Human/Animation/ABP_Human_ThirdPerson"));
	if (PlayerThirdPersonAnimBlueprintClass.Succeeded())
	{
		ThirdPersonMeshComponent->SetAnimClass(PlayerThirdPersonAnimBlueprintClass.Class);
	}
	
	static ConstructorHelpers::FClassFinder<ALootableRagdoll> PlayerRagdollBlueprint(TEXT("/Game/WildOmissionCore/Characters/Human/BP_Human_Ragdoll"));
	if (PlayerRagdollBlueprint.Succeeded())
	{
		RagdollClass = PlayerRagdollBlueprint.Class;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> FallCrunchSoundObject(TEXT("/Game/WildOmissionCore/Characters/Human/Audio/Damage/A_Human_Fall_Crunch"));
	if (FallCrunchSoundObject.Succeeded())
	{
		FallCrunchSound = FallCrunchSoundObject.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> SpookySoundObject(TEXT("/Game/WildOmissionCore/Audio/Spooky/SpookySound_Cue"));
	if (SpookySoundObject.Succeeded())
	{
		SpookySoundCue = SpookySoundObject.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultMappingContextBlueprint(TEXT("/Game/WildOmissionCore/Input/MC_DefaultMappingContext"));
	if (DefaultMappingContextBlueprint.Succeeded())
	{
		DefaultMappingContext = DefaultMappingContextBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> MoveForwardActionBlueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_MoveForward"));
	if (MoveForwardActionBlueprint.Succeeded())
	{
		MoveForwardAction = MoveForwardActionBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> MoveBackwardActionBlueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_MoveBackward"));
	if (MoveBackwardActionBlueprint.Succeeded())
	{
		MoveBackwardAction = MoveBackwardActionBlueprint.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<UInputAction> MoveLeftActionBlueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_MoveLeft"));
	if (MoveLeftActionBlueprint.Succeeded())
	{
		MoveLeftAction = MoveLeftActionBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> MoveRightActionBlueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_MoveRight"));
	if (MoveRightActionBlueprint.Succeeded())
	{
		MoveRightAction = MoveRightActionBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> LookActionBlueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_Look"));
	if (LookActionBlueprint.Succeeded())
	{
		LookAction = LookActionBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> SprintActionBlueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_Sprint"));
	if (SprintActionBlueprint.Succeeded())
	{
		SprintAction = SprintActionBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> CrouchActionBlueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_Crouch"));
	if (CrouchActionBlueprint.Succeeded())
	{
		CrouchAction = CrouchActionBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> JumpActionBlueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_Jump"));
	if (JumpActionBlueprint.Succeeded())
	{
		JumpAction = JumpActionBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> FlyActionBlueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_Fly"));
	if (FlyActionBlueprint.Succeeded())
	{
		FlyAction = FlyActionBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> PrimaryActionBlueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_Primary"));
	if (PrimaryActionBlueprint.Succeeded())
	{
		PrimaryAction = PrimaryActionBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> SecondaryActionBlueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_Secondary"));
	if (SecondaryActionBlueprint.Succeeded())
	{
		SecondaryAction = SecondaryActionBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InteractActionBlueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_Interact"));
	if (InteractActionBlueprint.Succeeded())
	{
		InteractAction = InteractActionBlueprint.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<UInputAction> ReloadActionBlueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_Reload"));
	if (ReloadActionBlueprint.Succeeded())
	{
		ReloadAction = ReloadActionBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> ToolbarSelectionIncrementBlueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_ToolbarSelectionIncrement"));
	if (ToolbarSelectionIncrementBlueprint.Succeeded())
	{
		ToolbarSelectionIncrementAction = ToolbarSelectionIncrementBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> ToolbarSelectionDecrementBlueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_ToolbarSelectionDecrement"));
	if (ToolbarSelectionDecrementBlueprint.Succeeded())
	{
		ToolbarSelectionDecrementAction = ToolbarSelectionDecrementBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> ToolbarSelection1Blueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_ToolbarSelect1"));
	if (ToolbarSelection1Blueprint.Succeeded())
	{
		ToolbarSelection1Action = ToolbarSelection1Blueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> ToolbarSelection2Blueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_ToolbarSelect2"));
	if (ToolbarSelection2Blueprint.Succeeded())
	{
		ToolbarSelection2Action = ToolbarSelection2Blueprint.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<UInputAction> ToolbarSelection3Blueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_ToolbarSelect3"));
	if (ToolbarSelection3Blueprint.Succeeded())
	{
		ToolbarSelection3Action = ToolbarSelection3Blueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> ToolbarSelection4Blueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_ToolbarSelect4"));
	if (ToolbarSelection4Blueprint.Succeeded())
	{
		ToolbarSelection4Action = ToolbarSelection4Blueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> ToolbarSelection5Blueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_ToolbarSelect5"));
	if (ToolbarSelection5Blueprint.Succeeded())
	{
		ToolbarSelection5Action = ToolbarSelection5Blueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> ToolbarSelection6Blueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_ToolbarSelect6"));
	if (ToolbarSelection6Blueprint.Succeeded())
	{
		ToolbarSelection6Action = ToolbarSelection6Blueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> ToggleInventoryMenuActionBlueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_ToggleInventoryMenu"));
	if (ToggleInventoryMenuActionBlueprint.Succeeded())
	{
		ToggleInventoryMenuAction = ToggleInventoryMenuActionBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> ToggleCraftingMenuActionBlueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_ToggleCraftingMenu"));
	if (ToggleCraftingMenuActionBlueprint.Succeeded())
	{
		ToggleCraftingMenuAction = ToggleCraftingMenuActionBlueprint.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> ToggleChatActionBlueprint(TEXT("/Game/WildOmissionCore/Input/InputActions/IA_ToggleChat"));
	if (ToggleChatActionBlueprint.Succeeded())
	{
		ToggleChatAction = ToggleChatActionBlueprint.Object;
	}

}

void AWildOmissionCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWildOmissionCharacter, DesiredMovementSpeed);
	DOREPLIFETIME_CONDITION(AWildOmissionCharacter, ReplicatedControlRotation, COND_SimulatedOnly);
}

void AWildOmissionCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	SetupEnhancedInputSubsystem();
	SetupMesh();
	SetupPlayerHUD();
	SetupGameMode();
	ApplyInputSettings();
	ApplyGameplaySettings();
	ApplyPostProcessingSettings();
	SetupLocalComponents();
	StopSprinting();

	TimeToNextSpookySound = FMath::RandRange(60.0f, 300.0f);

	if (EquipComponent && AimComponent)
	{
		EquipComponent->OnStartAiming.AddDynamic(AimComponent, &UPlayerAimComponent::StartAiming);
		EquipComponent->OnStopAiming.AddDynamic(AimComponent, &UPlayerAimComponent::StopAiming);
		AimComponent->OnAimStateChanged.AddDynamic(this, &AWildOmissionCharacter::RefreshDesiredMovementSpeed);
	}

	if (HasAuthority() && VitalsComponent)
	{
		VitalsComponent->OnHealthDepleted.AddDynamic(this, &AWildOmissionCharacter::HandleDeath);
	}
}

void AWildOmissionCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsLocallyControlled())
	{
		SpookyCounter += DeltaTime;
		if (SpookyCounter > TimeToNextSpookySound)
		{
			PlaySpookySound();
			SpookyCounter = 0;
		}
	}

	if (EquipComponent == nullptr)
	{
		return;
	}

	EquipComponent->UpdateControlRotation(GetReplicatedControlRotation());

	HandleUnderwater();

	if (!HasAuthority())
	{
		return;
	}

	ReplicatedControlRotation = GetControlRotation();

	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	if (CharacterMovementComponent == nullptr)
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	if (CurrentLocation.Z < 0.0f && !CharacterMovementComponent->IsSwimming() && !CharacterMovementComponent->IsFlying())
	{
		CharacterMovementComponent->SetMovementMode(EMovementMode::MOVE_Swimming);
	}
	else if (CurrentLocation.Z >= 0.0f && CharacterMovementComponent->IsSwimming())
	{
		CharacterMovementComponent->SetMovementMode(EMovementMode::MOVE_Walking);
	}

}

void AWildOmissionCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	SetupEnhancedInputSubsystem();
	SetupMesh();
	SetupPlayerHUD();
	SetupGameMode();
	ApplyInputSettings();
	ApplyGameplaySettings();
	ApplyPostProcessingSettings();
	SetupLocalComponents();
}

void AWildOmissionCharacter::UnPossessed()
{
	Super::UnPossessed();

	if (EquipComponent == nullptr)
	{
		return;
	}

	EquipComponent->DestroyEquipedItem();
}

void AWildOmissionCharacter::BeginDestroy()
{
	Super::BeginDestroy();
}

void AWildOmissionCharacter::Jump()
{
	Super::Jump();

	UWildOmissionGameUserSettings* UserSettings = UWildOmissionGameUserSettings::GetWildOmissionGameUserSettings();
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (IsLocallyControlled() && CanJump() && UserSettings && UserSettings->GetCameraShakeEnabled() && PlayerController && JumpCameraShake)
	{
		PlayerController->ClientStartCameraShake(JumpCameraShake);
	}
}

void AWildOmissionCharacter::Landed(const FHitResult& HitResult)
{
	Super::Landed(HitResult);

	if (!HasAuthority())
	{
		return;
	}

	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	if (CharacterMovementComponent == nullptr)
	{
		return;
	}

	float FallVelocity = CharacterMovementComponent->GetLastUpdateVelocity().GetAbs().Z;

	if (FallVelocity > 1000.0f)
	{
		float DamageToApply = FMath::Lerp(0.0f, 100.0f, (FallVelocity - 1000.0f) / 1000.0f);
		FPointDamageEvent FallDamageEvent(DamageToApply, HitResult, -FVector::UpVector, nullptr);
		TakeDamage(DamageToApply, FallDamageEvent, GetController(), this);
		Multi_PlayFallCrunchSound();
	}
}

void AWildOmissionCharacter::HandleFly()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	//OurPlayerState->GetUniqueId().ToString() != TEXT("76561198277223961"))
	AWildOmissionPlayerController* OurPlayerController = Cast<AWildOmissionPlayerController>(GetController());
	if (OurPlayerController == nullptr)
	{
		return;
	}

	if (!World->IsEditorWorld() && OurPlayerController->IsSurvivalMode())
	{
		return;
	}

	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	if (CharacterMovementComponent == nullptr)
	{
		return;
	}

	if (!HasAuthority())
	{
		Server_HandleFly();
	}

	const bool AlreadyFlying = CharacterMovementComponent->IsFlying();
	AlreadyFlying ? CharacterMovementComponent->SetMovementMode(EMovementMode::MOVE_Walking)
		: CharacterMovementComponent->SetMovementMode(EMovementMode::MOVE_Flying);
}

void AWildOmissionCharacter::SetupEnhancedInputSubsystem()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController == nullptr)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (Subsystem == nullptr)
	{
		return;
	}

	Subsystem->AddMappingContext(DefaultMappingContext, 0);
}

void AWildOmissionCharacter::SetupMesh()
{
	USkeletalMeshComponent* ThirdPersonMeshComponent = GetMesh();
	if (ThirdPersonMeshComponent)
	{
		ThirdPersonMeshComponent->SetVisibility(!IsLocallyControlled());
	}

	if (FirstPersonArmsMeshComponent)
	{
		FirstPersonArmsMeshComponent->SetVisibility(IsLocallyControlled());
	}
}

void AWildOmissionCharacter::SetupGameMode()
{
	AWildOmissionPlayerController* OwnerPlayerController = Cast<AWildOmissionPlayerController>(GetController());
	if (OwnerPlayerController == nullptr || OwnerPlayerController->IsSurvivalMode())
	{
		return;
	}

	if (VitalsComponent)
	{
		VitalsComponent->SetGodMode(true);
	}

	if (PlayerHUDWidget)
	{
		PlayerHUDWidget->SetVitalsHidden(true);
	}
}

void AWildOmissionCharacter::ApplyInputSettings()
{
	UWildOmissionGameUserSettings* UserSettings = UWildOmissionGameUserSettings::GetWildOmissionGameUserSettings();
	if (!IsLocallyControlled() || UserSettings == nullptr || DefaultMappingContext == nullptr)
	{
		return;
	}

	DefaultMappingContext->UnmapAll();
	LookUpInverted = UserSettings->GetInvertedMouseY();
	DefaultMappingContext->MapKey(MoveForwardAction, UserSettings->GetMoveForwardKey());
	DefaultMappingContext->MapKey(MoveBackwardAction, UserSettings->GetMoveBackwardKey());
	DefaultMappingContext->MapKey(MoveLeftAction, UserSettings->GetMoveLeftKey());
	DefaultMappingContext->MapKey(MoveRightAction, UserSettings->GetMoveRightKey());
	DefaultMappingContext->MapKey(LookAction, EKeys::Mouse2D);
	DefaultMappingContext->MapKey(SprintAction, UserSettings->GetSprintKey());
	DefaultMappingContext->MapKey(CrouchAction, UserSettings->GetCrouchKey());
	DefaultMappingContext->MapKey(JumpAction, UserSettings->GetJumpKey());
	DefaultMappingContext->MapKey(FlyAction, UserSettings->GetFlyKey());
	DefaultMappingContext->MapKey(PrimaryAction, UserSettings->GetPrimaryKey());
	DefaultMappingContext->MapKey(SecondaryAction, UserSettings->GetSecondaryKey());
	DefaultMappingContext->MapKey(InteractAction, UserSettings->GetInteractKey());
	DefaultMappingContext->MapKey(ReloadAction, UserSettings->GetReloadKey());
	DefaultMappingContext->MapKey(ToolbarSelectionIncrementAction, EKeys::MouseScrollDown);
	DefaultMappingContext->MapKey(ToolbarSelectionDecrementAction, EKeys::MouseScrollUp);
	DefaultMappingContext->MapKey(ToolbarSelection1Action, EKeys::One);
	DefaultMappingContext->MapKey(ToolbarSelection2Action, EKeys::Two);
	DefaultMappingContext->MapKey(ToolbarSelection3Action, EKeys::Three);
	DefaultMappingContext->MapKey(ToolbarSelection4Action, EKeys::Four);
	DefaultMappingContext->MapKey(ToolbarSelection5Action, EKeys::Five);
	DefaultMappingContext->MapKey(ToolbarSelection6Action, EKeys::Six);
	DefaultMappingContext->MapKey(ToggleInventoryMenuAction, UserSettings->GetInventoryKey());
	DefaultMappingContext->MapKey(ToggleCraftingMenuAction, UserSettings->GetCraftingKey());
	DefaultMappingContext->MapKey(ToggleChatAction, UserSettings->GetChatKey());
}

void AWildOmissionCharacter::ApplyGameplaySettings()
{
	UWildOmissionGameUserSettings* UserSettings = UWildOmissionGameUserSettings::GetWildOmissionGameUserSettings();
	if (!IsLocallyControlled() || UserSettings == nullptr
		|| FirstPersonCameraComponent == nullptr
		|| ChunkInvokerComponent == nullptr || PlayerHUDWidget == nullptr)
	{
		return;
	}

	FirstPersonCameraComponent->SetFieldOfView(UserSettings->GetFieldOfView());
	ChunkInvokerComponent->Server_SetRenderDistance(UserSettings->GetRenderDistance());
	PlayerHUDWidget->ShowBranding(UserSettings->GetShowBranding());
	PlayerHUDWidget->ShowCrosshair(UserSettings->GetShowCrosshair());
	PlayerHUDWidget->SetHideChatUnlessOpen(UserSettings->GetHideChatUnlessOpen());
	PlayerHUDWidget->SetVisibility(UserSettings->GetHideHUD() ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
}

void AWildOmissionCharacter::ApplyPostProcessingSettings()
{
	UWildOmissionGameUserSettings* UserSettings = UWildOmissionGameUserSettings::GetWildOmissionGameUserSettings();
	if (!IsLocallyControlled() || UserSettings == nullptr || FirstPersonCameraComponent == nullptr)
	{
		return;
	}
	const float GammaValue = UserSettings->GetGamma() / 100.0f;
	FirstPersonCameraComponent->PostProcessSettings.ColorGamma = FVector4(GammaValue, GammaValue, GammaValue, GammaValue);
	FirstPersonCameraComponent->PostProcessSettings.AutoExposureMinBrightness = UserSettings->GetAutoExposureEnabled() ? -10.0f : 1.0f;
	FirstPersonCameraComponent->PostProcessSettings.AutoExposureMaxBrightness = UserSettings->GetAutoExposureEnabled() ? 20.0f : 1.0f;
	FirstPersonCameraComponent->PostProcessSettings.MotionBlurAmount = UserSettings->GetMotionBlurEnabled() ? 0.3f : 0.0f;
	FirstPersonCameraComponent->PostProcessSettings.BloomIntensity = UserSettings->GetBloomEnabled() ? 0.675f : 0.0f;
	FirstPersonCameraComponent->PostProcessSettings.AmbientOcclusionIntensity = UserSettings->GetAmbientOcclusionEnabled() ? 0.5f : 0.0f;
	FirstPersonCameraComponent->PostProcessSettings.FilmGrainIntensity = UserSettings->GetFilmGrainEnabled() ? 0.5f : 0.0f;
}

void AWildOmissionCharacter::SetupPlayerHUD()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!IsLocallyControlled() || PlayerController == nullptr || PlayerHUDWidget != nullptr || PlayerHUDWidgetClass == nullptr)
	{
		return;
	}

	PlayerHUDWidget = CreateWidget<UPlayerHUDWidget>(PlayerController, PlayerHUDWidgetClass);
	if (PlayerHUDWidget == nullptr)
	{
		return;
	}

	PlayerHUDWidget->AddToViewport();
}

void AWildOmissionCharacter::SetupLocalComponents()
{
	if (!IsLocallyControlled() || SpecialEffectsManagerComponent != nullptr)
	{
		return;
	}

	SpecialEffectsManagerComponent = NewObject<USpecialEffectsManagerComponent>(this, USpecialEffectsManagerComponent::StaticClass(), TEXT("SpecialEffectsManagerComponent"));
	if (SpecialEffectsManagerComponent == nullptr)
	{
		return;
	}

	SpecialEffectsManagerComponent->RegisterComponent();
	SpecialEffectsManagerComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

}

void AWildOmissionCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// UnOccupy any open container
	if (InventoryManipulatorComponent && InventoryManipulatorComponent->GetOpenContainer())
	{
		AItemContainerBase* OpenItemContainer = Cast<AItemContainerBase>(InventoryManipulatorComponent->GetOpenContainer()->GetOwner());
		if (OpenItemContainer)
		{
			OpenItemContainer->UnOccupy();
		}
	}

	// Remove the hud
	if (PlayerHUDWidget)
	{
		PlayerHUDWidget->RemoveFromParent();
		PlayerHUDWidget = nullptr;
	}
}

void AWildOmissionCharacter::HandleDeath()
{
	UWorld* World = GetWorld();
	AWildOmissionPlayerController* OurController = Cast<AWildOmissionPlayerController>(Controller);
	if (World == nullptr || OurController == nullptr)
	{
		return;
	}

	OurController->Save();
	OurController->Client_ShowDeathMenu();

	// Game chat manager send death message
	AWildOmissionGameMode* WildOmissionGameMode = Cast<AWildOmissionGameMode>(World->GetAuthGameMode());
	if (WildOmissionGameMode == nullptr)
	{
		return;
	}

	// Get the GameChatManager from the GameMode
	AGameChatManager* GameChatManager = WildOmissionGameMode->GetGameChatManager();
	if (GameChatManager == nullptr)
	{
		return;
	}

	// Send the death notification
	GameChatManager->SendMessage(this->GetPlayerState(), TEXT("has died."), EChatMessageType::DEATH_NOTIFICTION);

	// Get all attached actors
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);

	// Create lootable container with inventory
	ALootableRagdoll* SpawnedRagdoll = World->SpawnActor<ALootableRagdoll>(RagdollClass, GetActorTransform());
	if (SpawnedRagdoll == nullptr)
	{
		return;
	}

	USkeletalMeshComponent* RagdollSkeletalMeshComponent = SpawnedRagdoll->GetMeshComponent();
	if (RagdollSkeletalMeshComponent)
	{
		for (AActor* AttachedActor : AttachedActors)
		{
			if (AttachedActor == nullptr)
			{
				continue;
			}

			const FName Socket = AttachedActor->GetAttachParentSocketName();
			AttachedActor->AttachToComponent(RagdollSkeletalMeshComponent, FAttachmentTransformRules::KeepRelativeTransform, Socket);
		}
	}

	// Set Items to be this players items
	UInventoryComponent* RagdollInventoryComponent = SpawnedRagdoll->GetInventoryComponent();
	if (RagdollInventoryComponent == nullptr)
	{
		return;
	}

	RagdollInventoryComponent->Load(InventoryComponent->Save());
	
	if (OnPlayerDeath.IsBound())
	{
		OnPlayerDeath.Broadcast(this->GetActorLocation());
	}

	AController* LastDamageCauser = LastHitBy;
	if (IsValid(LastDamageCauser))
	{
		if (AWildOmissionCharacter* LastDamageCauserCharacter = Cast<AWildOmissionCharacter>(LastDamageCauser->GetPawn()))
		{
			if (LastDamageCauserCharacter->OnKilledAnotherPlayerSignature.IsBound())
			{
				LastDamageCauserCharacter->OnKilledAnotherPlayerSignature.Broadcast(Cast<APlayerController>(this->GetController()));
			}
		}
	}

	this->Destroy();
}

void AWildOmissionCharacter::SetGodMode(bool GodMode)
{
	VitalsComponent->SetGodMode(GodMode);
}

void AWildOmissionCharacter::HandleUnderwater()
{
	if (FirstPersonCameraComponent->GetComponentLocation().Z < 0.0f)
	{
		bUnderwater = true;
		FirstPersonCameraComponent->PostProcessSettings.SceneColorTint = FLinearColor(0.1f, 0.6f, 1.0f, 1.0f);
	}
	else
	{
		bUnderwater = false;
		FirstPersonCameraComponent->PostProcessSettings.SceneColorTint = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

void AWildOmissionCharacter::PlaySpookySound()
{
	if (!IsLocallyControlled() || !UKismetMathLibrary::RandomBoolWithWeight(0.25f))
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(GetWorld(), SpookySoundCue, this->GetActorLocation());
}

//********************************
// Input
//********************************

void AWildOmissionCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Cast PlayerInputComponent to UEnhancedPlayerInputComponent
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent == nullptr)
	{
		return;
	}

	// Bind function callbacks to input actions
	EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AWildOmissionCharacter::MoveForward);
	EnhancedInputComponent->BindAction(MoveBackwardAction, ETriggerEvent::Triggered, this, &AWildOmissionCharacter::MoveBackward);
	EnhancedInputComponent->BindAction(MoveLeftAction, ETriggerEvent::Triggered, this, &AWildOmissionCharacter::MoveLeft);
	EnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AWildOmissionCharacter::MoveRight);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AWildOmissionCharacter::Look);
	EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AWildOmissionCharacter::OnSprintButtonPressed);
	EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AWildOmissionCharacter::OnSprintButtonReleased);
	EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AWildOmissionCharacter::StartCrouching);
	EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AWildOmissionCharacter::StopCrouching);
	EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AWildOmissionCharacter::OnCrouchHeld);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AWildOmissionCharacter::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AWildOmissionCharacter::OnJumpHeld);
	EnhancedInputComponent->BindAction(FlyAction, ETriggerEvent::Started, this, &AWildOmissionCharacter::HandleFly);

	EnhancedInputComponent->BindAction(PrimaryAction, ETriggerEvent::Started, this, &AWildOmissionCharacter::PrimaryPressed);
	EnhancedInputComponent->BindAction(PrimaryAction, ETriggerEvent::Completed, this, &AWildOmissionCharacter::PrimaryReleased);
	EnhancedInputComponent->BindAction(SecondaryAction, ETriggerEvent::Started, this, &AWildOmissionCharacter::SecondaryPressed);
	EnhancedInputComponent->BindAction(SecondaryAction, ETriggerEvent::Completed, this, &AWildOmissionCharacter::SecondaryReleased);
	EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, InteractionComponent, &UInteractionComponent::Interact);
	EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &AWildOmissionCharacter::ReloadPressed);
	EnhancedInputComponent->BindAction(ToolbarSelectionIncrementAction, ETriggerEvent::Started, this, &AWildOmissionCharacter::ToolbarSelectionIncrement);
	EnhancedInputComponent->BindAction(ToolbarSelectionDecrementAction, ETriggerEvent::Started, this, &AWildOmissionCharacter::ToolbarSelectionDecrement);
	EnhancedInputComponent->BindAction(ToolbarSelection1Action, ETriggerEvent::Started, this, &AWildOmissionCharacter::SelectToolbarSlot1);
	EnhancedInputComponent->BindAction(ToolbarSelection2Action, ETriggerEvent::Started, this, &AWildOmissionCharacter::SelectToolbarSlot2);
	EnhancedInputComponent->BindAction(ToolbarSelection3Action, ETriggerEvent::Started, this, &AWildOmissionCharacter::SelectToolbarSlot3);
	EnhancedInputComponent->BindAction(ToolbarSelection4Action, ETriggerEvent::Started, this, &AWildOmissionCharacter::SelectToolbarSlot4);
	EnhancedInputComponent->BindAction(ToolbarSelection5Action, ETriggerEvent::Started, this, &AWildOmissionCharacter::SelectToolbarSlot5);
	EnhancedInputComponent->BindAction(ToolbarSelection6Action, ETriggerEvent::Started, this, &AWildOmissionCharacter::SelectToolbarSlot6);
	EnhancedInputComponent->BindAction(ToggleInventoryMenuAction, ETriggerEvent::Started, this, &AWildOmissionCharacter::ToggleInventoryMenu);
	EnhancedInputComponent->BindAction(ToggleCraftingMenuAction, ETriggerEvent::Started, this, &AWildOmissionCharacter::ToggleCraftingMenu);
	EnhancedInputComponent->BindAction(ToggleChatAction, ETriggerEvent::Started, this, &AWildOmissionCharacter::ToggleChat);
}

void AWildOmissionCharacter::MoveForward()
{
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	if (CharacterMovementComponent == nullptr)
	{
		return;
	}

	if (bSprintButtonHeld && !IsSprinting())
	{
		StartSprinting();
	}

	if (!CharacterMovementComponent->IsSwimming())
	{
		AddMovementInput(GetActorForwardVector(), 1.0f);
	}
	else
	{
		AddMovementInput(UKismetMathLibrary::GetForwardVector(GetControlRotation()), 1.0f);
	}
}

void AWildOmissionCharacter::MoveBackward()
{
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	if (CharacterMovementComponent == nullptr)
	{
		return;
	}

	if (IsSprinting())
	{
		StopSprinting();
	}

	if (!CharacterMovementComponent->IsSwimming())
	{
		AddMovementInput(GetActorForwardVector(), -1.0f);
	}
	else
	{
		AddMovementInput(UKismetMathLibrary::GetForwardVector(GetControlRotation()), -1.0f);
	}
}

void AWildOmissionCharacter::MoveLeft()
{
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	if (CharacterMovementComponent == nullptr)
	{
		return;
	}

	if (IsSprinting())
	{
		StopSprinting();
	}

	if (!CharacterMovementComponent->IsSwimming())
	{
		AddMovementInput(GetActorRightVector(), -1.0f);
	}
	else
	{
		
		AddMovementInput(UKismetMathLibrary::GetRightVector(GetControlRotation()), -1.0f);
	}
}

void AWildOmissionCharacter::MoveRight()
{
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	if (CharacterMovementComponent == nullptr)
	{
		return;
	}

	if (IsSprinting())
	{
		StopSprinting();
	}

	if (!CharacterMovementComponent->IsSwimming())
	{
		AddMovementInput(GetActorRightVector(), 1.0f);
	}
	else
	{
		AddMovementInput(UKismetMathLibrary::GetRightVector(GetControlRotation()), 1.0f);
	}
}

void AWildOmissionCharacter::Look(const FInputActionValue& Value)
{
	if (PlayerHUDWidget == nullptr || PlayerHUDWidget->IsMenuOpen() || AimComponent == nullptr)
	{
		return;
	}

	FVector2D LookAxis = Value.Get<FVector2D>();

	const float LookSensitivity = AimComponent->GetLookSensitivity();

	AddControllerYawInput(LookAxis.X * LookSensitivity);
	const float Invert = LookUpInverted ? 1.0f : -1.0f;
	AddControllerPitchInput((LookAxis.Y * LookSensitivity) * Invert);
}

void AWildOmissionCharacter::OnSprintButtonPressed()
{
	bSprintButtonHeld = true;
	
	StartSprinting();
}

void AWildOmissionCharacter::OnSprintButtonReleased()
{
	bSprintButtonHeld = false;

	StopSprinting();
}

void AWildOmissionCharacter::StartSprinting()
{
	if (PlayerHUDWidget && PlayerHUDWidget->IsMenuOpen())
	{
		return;
	}

	Server_Sprint(true);
	bSprinting = true;
	RefreshDesiredMovementSpeed();
}

void AWildOmissionCharacter::StopSprinting()
{
	if (PlayerHUDWidget && PlayerHUDWidget->IsMenuOpen())
	{
		return;
	}

	Server_Sprint(false);
	bSprinting = false;
	RefreshDesiredMovementSpeed();
}

void AWildOmissionCharacter::StartCrouching()
{
	if (PlayerHUDWidget && PlayerHUDWidget->IsMenuOpen())
	{
		return;
	}

	Crouch();
}

void AWildOmissionCharacter::StopCrouching()
{
	if (PlayerHUDWidget && PlayerHUDWidget->IsMenuOpen())
	{
		return;
	}

	UnCrouch();
}

void AWildOmissionCharacter::OnJumpHeld()
{
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	if (CharacterMovementComponent == nullptr || !CharacterMovementComponent->IsFlying())
	{
		return;
	}

	AddMovementInput(FVector::UpVector);
}

void AWildOmissionCharacter::OnCrouchHeld()
{
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	if (CharacterMovementComponent == nullptr || !CharacterMovementComponent->IsFlying())
	{
		return;
	}

	AddMovementInput(-FVector::UpVector);
}

void AWildOmissionCharacter::OnRep_MovementSpeed()
{
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	if (CharacterMovementComponent == nullptr)
	{
		return;
	}

	CharacterMovementComponent->MaxWalkSpeed = DesiredMovementSpeed;
}

void AWildOmissionCharacter::RefreshDesiredMovementSpeed()
{
	if (IsAiming())
	{
		DesiredMovementSpeed = AimMovementSpeed;
	}
	else if (IsSprinting())
	{
		DesiredMovementSpeed = SprintMovementSpeed;
	}
	else
	{
		DesiredMovementSpeed = WalkMovementSpeed;
	}

	OnRep_MovementSpeed();
}

bool AWildOmissionCharacter::Server_HandleFly_Validate()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	AWildOmissionPlayerController* OurPlayerController = Cast<AWildOmissionPlayerController>(GetController());
	if (OurPlayerController == nullptr)
	{
		return false;;
	}

	if (!World->IsEditorWorld() && OurPlayerController->IsSurvivalMode())
	{
		return false;
	}

	return true;
}

void AWildOmissionCharacter::Server_HandleFly_Implementation()
{
	HandleFly();
}

void AWildOmissionCharacter::Server_Sprint_Implementation(bool bShouldSprint)
{
	bSprinting = bShouldSprint;
	RefreshDesiredMovementSpeed();
}

void AWildOmissionCharacter::Multi_PlayFallCrunchSound_Implementation()
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), FallCrunchSound, this->GetActorLocation());
}

void AWildOmissionCharacter::PrimaryPressed()
{
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	if (PlayerHUDWidget == nullptr || PlayerHUDWidget->IsMenuOpen() || !EquipComponent->PrimaryEnabled()
		|| CharacterMovementComponent == nullptr || CharacterMovementComponent->IsSwimming())
	{
		return;
	}

	EquipComponent->PrimaryPressed();
}

void AWildOmissionCharacter::PrimaryReleased()
{
	EquipComponent->PrimaryReleased();
}

void AWildOmissionCharacter::SecondaryPressed()
{
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	if (PlayerHUDWidget == nullptr || PlayerHUDWidget->IsMenuOpen() || !EquipComponent->SecondaryEnabled()
		|| CharacterMovementComponent == nullptr || CharacterMovementComponent->IsSwimming())
	{
		return;
	}

	EquipComponent->SecondaryPressed();
}

void AWildOmissionCharacter::SecondaryReleased()
{
	EquipComponent->SecondaryReleased();
}

void AWildOmissionCharacter::ReloadPressed()
{
	EquipComponent->ReloadPressed();
}

void AWildOmissionCharacter::ToggleInventoryMenu()
{
	if (PlayerHUDWidget == nullptr || PlayerHUDWidget->IsChatMenuOpen())
	{
		return;
	}

	EquipComponent->PrimaryReleased();
	EquipComponent->SecondaryReleased();

	PlayerHUDWidget->ToggleInventoryMenu();
}

void AWildOmissionCharacter::ToggleCraftingMenu()
{
	if (PlayerHUDWidget == nullptr || PlayerHUDWidget->IsChatMenuOpen())
	{
		return;
	}

	EquipComponent->PrimaryReleased();
	EquipComponent->SecondaryReleased();

	PlayerHUDWidget->ToggleCraftingMenu();
}

void AWildOmissionCharacter::ToggleChat()
{
	if (PlayerHUDWidget == nullptr || PlayerHUDWidget->IsMenuOpen())
	{
		return;
	}

	EquipComponent->PrimaryReleased();
	EquipComponent->SecondaryReleased();

	PlayerHUDWidget->ToggleChatMenu();
}

void AWildOmissionCharacter::ToolbarSelectionIncrement()
{
	if (PlayerHUDWidget == nullptr || PlayerHUDWidget->IsMenuOpen())
	{
		return;
	}

	InventoryComponent->IncrementToolbarSelection();
}

void AWildOmissionCharacter::ToolbarSelectionDecrement()
{
	if (PlayerHUDWidget == nullptr || PlayerHUDWidget->IsMenuOpen())
	{
		return;
	}

	InventoryComponent->DecrementToolbarSelection();
}

void AWildOmissionCharacter::SelectToolbarSlot1()
{
	if (PlayerHUDWidget == nullptr || PlayerHUDWidget->IsMenuOpen())
	{
		return;
	}

	InventoryComponent->SetToolbarSelectionIndex(0);
}

void AWildOmissionCharacter::SelectToolbarSlot2()
{
	if (PlayerHUDWidget == nullptr || PlayerHUDWidget->IsMenuOpen())
	{
		return;
	}

	InventoryComponent->SetToolbarSelectionIndex(1);
}

void AWildOmissionCharacter::SelectToolbarSlot3()
{
	if (PlayerHUDWidget == nullptr || PlayerHUDWidget->IsMenuOpen())
	{
		return;
	}

	InventoryComponent->SetToolbarSelectionIndex(2);
}

void AWildOmissionCharacter::SelectToolbarSlot4()
{
	if (PlayerHUDWidget == nullptr || PlayerHUDWidget->IsMenuOpen())
	{
		return;
	}

	InventoryComponent->SetToolbarSelectionIndex(3);
}

void AWildOmissionCharacter::SelectToolbarSlot5()
{
	if (PlayerHUDWidget == nullptr || PlayerHUDWidget->IsMenuOpen())
	{
		return;
	}

	InventoryComponent->SetToolbarSelectionIndex(4);
}

void AWildOmissionCharacter::SelectToolbarSlot6()
{
	if (PlayerHUDWidget == nullptr || PlayerHUDWidget->IsMenuOpen())
	{
		return;
	}

	InventoryComponent->SetToolbarSelectionIndex(5);
}

//********************************
// Getters
//********************************

USkeletalMeshComponent* AWildOmissionCharacter::GetArmsMesh() const
{
	return FirstPersonArmsMeshComponent;
}

float AWildOmissionCharacter::GetHeadPitch() const
{
	return GetReplicatedControlRotation().GetNormalized().Pitch;
}

FRotator AWildOmissionCharacter::GetReplicatedControlRotation() const
{
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		return ReplicatedControlRotation;
	}
	
	return GetControlRotation();
}

bool AWildOmissionCharacter::IsSprinting() const
{
	return bSprinting;
}

bool AWildOmissionCharacter::IsAiming() const
{
	return AimComponent->IsAiming();
}

bool AWildOmissionCharacter::IsUnderwater() const
{
	return bUnderwater;
}

UCameraComponent* AWildOmissionCharacter::GetFirstPersonCameraComponent()
{
	return FirstPersonCameraComponent;
}

UPlayerHUDWidget* AWildOmissionCharacter::GetHUDWidget() const
{
	return PlayerHUDWidget;
}

UVitalsComponent* AWildOmissionCharacter::GetVitalsComponent() const
{
	return VitalsComponent;
}

UInventoryManipulatorComponent* AWildOmissionCharacter::GetInventoryManipulatorComponent() const
{
	return InventoryManipulatorComponent;
}

UPlayerInventoryComponent* AWildOmissionCharacter::GetInventoryComponent() const
{
	return InventoryComponent;
}

UCraftingComponent* AWildOmissionCharacter::GetCraftingComponent() const
{
	return CraftingComponent;
}
