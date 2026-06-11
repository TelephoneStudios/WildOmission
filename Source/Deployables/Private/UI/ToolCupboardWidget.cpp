// Copyright Telephone Studios. All Rights Reserved.


#include "UI/ToolCupboardWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"
#include "Components/BuilderComponent.h"
#include "Deployables/ToolCupboard.h"

UToolCupboardWidget::UToolCupboardWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	SetIsFocusable(true);

	AuthorizeButton = nullptr;
	AuthorizeTextBlock = nullptr;
	ClearAuthorizedButton = nullptr;
	CloseButton = nullptr;
}

void UToolCupboardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AuthorizeButton->OnClicked.AddDynamic(this, &UToolCupboardWidget::OnAuthorizeButtonClicked);
	ClearAuthorizedButton->OnClicked.AddDynamic(this, &UToolCupboardWidget::OnClearAuthorizedButtonClicked);
	CloseButton->OnClicked.AddDynamic(this, &UToolCupboardWidget::Teardown);
}

void UToolCupboardWidget::Setup(AToolCupboard* InToolCupboard)
{
	ToolCupboard = InToolCupboard;

	APlayerController* PlayerController = GetOwningPlayer();
	if (PlayerController == nullptr)
	{
		return;
	}

	this->AddToViewport();

	RefreshUI();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(this->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(true);
}

void UToolCupboardWidget::Teardown()
{
	ToolCupboard = nullptr;

	APlayerController* PlayerController = GetOwningPlayer();
	if (PlayerController == nullptr)
	{
		return;
	}

	FInputModeGameOnly InputMode;
	
	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(false);

	if (OnTeardown.IsBound())
	{
		OnTeardown.Broadcast();
	}

	this->RemoveFromParent();
}

FReply UToolCupboardWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	Super::NativeOnKeyDown(InGeometry, InKeyEvent);

	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		this->Teardown();
	}

	return FReply::Handled();
}

bool UToolCupboardWidget::AreWeAuthorized() const
{
	APlayerState* OwnerPlayerState = GetOwningPlayerState();
	if (ToolCupboard == nullptr || OwnerPlayerState == nullptr)
	{
		return false;
	}

	return ToolCupboard->IsPlayerAuthorized(OwnerPlayerState->GetUniqueId()->ToString());
}

void UToolCupboardWidget::RefreshUI()
{
	if (ToolCupboard == nullptr)
	{
		return;
	}
	const bool HasAuthorization = AreWeAuthorized();

	const FString AuthorizeString = HasAuthorization ? TEXT("Deauthorize") : TEXT("Authorize");
	AuthorizeTextBlock->SetText(FText::FromString(AuthorizeString));
	
	const ESlateVisibility ClearAuthVisibility = HasAuthorization ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	ClearAuthorizedButton->SetIsEnabled(HasAuthorization);
	ClearAuthorizedButton->SetVisibility(ClearAuthVisibility);
}

void UToolCupboardWidget::OnAuthorizeButtonClicked()
{
	APawn* OwnerPawn = GetOwningPlayerPawn();
	if (OwnerPawn == nullptr)
	{
		return;
	}

	UBuilderComponent* OwnerBuilderComponent = OwnerPawn->FindComponentByClass<UBuilderComponent>();
	if (OwnerBuilderComponent == nullptr)
	{
		return;
	}

	AreWeAuthorized() ? OwnerBuilderComponent->Server_Deauthorize(ToolCupboard) : OwnerBuilderComponent->Server_Authorize(ToolCupboard);

	this->Teardown();
}

void UToolCupboardWidget::OnClearAuthorizedButtonClicked()
{
	APawn* OwnerPawn = GetOwningPlayerPawn();
	if (OwnerPawn == nullptr)
	{
		return;
	}

	UBuilderComponent* OwnerBuilderComponent = OwnerPawn->FindComponentByClass<UBuilderComponent>();
	if (OwnerBuilderComponent == nullptr)
	{
		return;
	}

	OwnerBuilderComponent->Server_ClearAllAuthorized(ToolCupboard);

	this->Teardown();
}
