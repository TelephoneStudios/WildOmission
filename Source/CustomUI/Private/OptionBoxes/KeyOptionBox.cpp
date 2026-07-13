// Copyright Telephone Studios. All Rights Reserved.


#include "OptionBoxes/KeyOptionBox.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Color/UIColors.h"

UKeyOptionBox::UKeyOptionBox(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	Border = nullptr;
	TextBlock = nullptr;

	SelectedKey = FKey();
	AwaitingInput = false;
}

void UKeyOptionBox::NativeConstruct()
{
	Super::NativeConstruct();

	Border->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f));
	AwaitingInput = false;
	SelectedKey = EKeys::Semicolon;
	RefreshTextBlock();
}

void UKeyOptionBox::SetSelectedKey(const FKey& NewSelectedKey)
{
	SelectedKey = NewSelectedKey;
	RefreshTextBlock();

	if (OnValueChanged.IsBound())
	{
		OnValueChanged.Broadcast(SelectedKey);
	}

	if (OnValueChangedNoParams.IsBound())
	{
		OnValueChangedNoParams.Broadcast();
	}
}

FKey UKeyOptionBox::GetSelectedKey() const
{
	return SelectedKey;
}

void UKeyOptionBox::NativeOnFocusLost(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnFocusLost(InFocusEvent);

	if (AwaitingInput == false)
	{
		return;
	}

	AwaitingInput = false;
	RefreshTextBlock();
}

FReply UKeyOptionBox::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	Super::NativeOnKeyDown(InGeometry, InKeyEvent);

	if (AwaitingInput == false)
	{
		return FReply::Handled();
	}
	
	AwaitingInput = false;
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		RefreshTextBlock();
		return FReply::Handled();
	}

	SetSelectedKey(InKeyEvent.GetKey());
	
	return FReply::Handled();
}

FReply UKeyOptionBox::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (AwaitingInput == false && InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		SetFocus();
		AwaitingInput = true;
		RefreshTextBlock();
		return FReply::Handled();
	}

	if (AwaitingInput)
	{
		AwaitingInput = false;
		SetSelectedKey(InMouseEvent.GetEffectingButton());
	}

	return FReply::Handled();
}

void UKeyOptionBox::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	FUIColor* LightGray = UUIColors::GetBaseColor(TEXT("LightGray"));
	if (LightGray == nullptr)
	{
		return;
	}
	FLinearColor HoveredColor = LightGray->Default;
	HoveredColor.A = 0.2f;
	Border->SetBrushColor(HoveredColor);
}

void UKeyOptionBox::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	Border->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f));
}

void UKeyOptionBox::RefreshTextBlock()
{
	FString DisplayString = SelectedKey.ToString();

	if (AwaitingInput)
	{
		DisplayString = TEXT("Press Desired Key.");
	}

	TextBlock->SetText(FText::FromString(DisplayString));
}
