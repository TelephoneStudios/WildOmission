// Copyright Telephone Studios. All Rights Reserved.


#include "ControlsSettingsWidget.h"
#include "OptionBoxes/CheckOptionBox.h"
#include "OptionBoxes/KeyOptionBox.h"
#include "OptionBoxes/SliderOptionBox.h"
#include "Interfaces/CharacterSettingsInterface.h"
#include "WildOmissionGameUserSettings.h"

UControlsSettingsWidget::UControlsSettingsWidget(const FObjectInitializer& ObjectInitializer) : USettingsCategoryWidget(ObjectInitializer)
{
	InvertMouseYCheckOptionBox = nullptr;
	AlphaToolbarScrollCheckOptionBox = nullptr;
	MouseSensitivitySliderOptionBox = nullptr;
	MoveForwardKeyOptionBox = nullptr;
	MoveBackwardKeyOptionBox = nullptr;
	MoveLeftKeyOptionBox = nullptr;
	MoveRightKeyOptionBox = nullptr;
	SprintKeyOptionBox = nullptr;
	CrouchKeyOptionBox = nullptr;
	JumpKeyOptionBox = nullptr;
	FlyKeyOptionBox = nullptr;

	PrimaryKeyOptionBox = nullptr;
	SecondaryKeyOptionBox = nullptr;
	InteractKeyOptionBox = nullptr;
	ReloadKeyOptionBox = nullptr;

	InventoryKeyOptionBox = nullptr;
	CraftingKeyOptionBox = nullptr;
	ChatKeyOptionBox = nullptr;
}

void UControlsSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	MouseSensitivitySliderOptionBox->SetMinValue(1.0f);
	MouseSensitivitySliderOptionBox->SetMaxValue(20.0f);	
}

void UControlsSettingsWidget::OnApply()
{
	Super::OnApply();

	UWildOmissionGameUserSettings* UserSettings = UWildOmissionGameUserSettings::GetWildOmissionGameUserSettings();
	if (UserSettings == nullptr)
	{
		return;
	}

	const float MouseSensitivity = FMath::RoundToInt32(MouseSensitivitySliderOptionBox->GetValue()) / 10.0f;

	UserSettings->SetInvertedMouseY(InvertMouseYCheckOptionBox->IsChecked());
	UserSettings->SetMouseSensitivity(MouseSensitivity);
	UserSettings->SetAlphaToolbarScroll(AlphaToolbarScrollCheckOptionBox->IsChecked());

	UserSettings->SetMoveForwardKey(MoveForwardKeyOptionBox->GetSelectedKey());
	UserSettings->SetMoveBackwardKey(MoveBackwardKeyOptionBox->GetSelectedKey());
	UserSettings->SetMoveLeftKey(MoveLeftKeyOptionBox->GetSelectedKey());
	UserSettings->SetMoveRightKey(MoveRightKeyOptionBox->GetSelectedKey());
	UserSettings->SetSprintKey(SprintKeyOptionBox->GetSelectedKey());
	UserSettings->SetCrouchKey(CrouchKeyOptionBox->GetSelectedKey());
	UserSettings->SetJumpKey(JumpKeyOptionBox->GetSelectedKey());
	UserSettings->SetFlyKey(FlyKeyOptionBox->GetSelectedKey());

	UserSettings->SetPrimaryKey(PrimaryKeyOptionBox->GetSelectedKey());
	UserSettings->SetSecondaryKey(SecondaryKeyOptionBox->GetSelectedKey());
	UserSettings->SetInteractKey(InteractKeyOptionBox->GetSelectedKey());
	UserSettings->SetReloadKey(ReloadKeyOptionBox->GetSelectedKey());

	UserSettings->SetInventoryKey(InventoryKeyOptionBox->GetSelectedKey());
	UserSettings->SetCraftingKey(CraftingKeyOptionBox->GetSelectedKey());
	UserSettings->SetChatKey(ChatKeyOptionBox->GetSelectedKey());

	ICharacterSettingsInterface* CharacterSettingsInterface = Cast<ICharacterSettingsInterface>(GetOwningPlayerPawn());
	if (CharacterSettingsInterface == nullptr)
	{
		return;
	}

	CharacterSettingsInterface->ApplyInputSettings();

	UserSettings->ApplySettings(false);

	OnRefresh();
}

void UControlsSettingsWidget::OnRefresh()
{
	Super::OnRefresh();
	
	InvertMouseYCheckOptionBox->OnValueChangedNoParams.RemoveDynamic(this, &UControlsSettingsWidget::OnApply);
	MouseSensitivitySliderOptionBox->OnValueChangedNoParams.RemoveDynamic(this, &UControlsSettingsWidget::OnApply);
	AlphaToolbarScrollCheckOptionBox->OnValueChangedNoParams.RemoveDynamic(this, &UControlsSettingsWidget::OnApply);
	MoveForwardKeyOptionBox->OnValueChangedNoParams.RemoveDynamic(this, &UControlsSettingsWidget::OnApply);
	MoveBackwardKeyOptionBox->OnValueChangedNoParams.RemoveDynamic(this, &UControlsSettingsWidget::OnApply);
	MoveLeftKeyOptionBox->OnValueChangedNoParams.RemoveDynamic(this, &UControlsSettingsWidget::OnApply);
	MoveRightKeyOptionBox->OnValueChangedNoParams.RemoveDynamic(this, &UControlsSettingsWidget::OnApply);
	SprintKeyOptionBox->OnValueChangedNoParams.RemoveDynamic(this, &UControlsSettingsWidget::OnApply);
	CrouchKeyOptionBox->OnValueChangedNoParams.RemoveDynamic(this, &UControlsSettingsWidget::OnApply);
	JumpKeyOptionBox->OnValueChangedNoParams.RemoveDynamic(this, &UControlsSettingsWidget::OnApply);
	FlyKeyOptionBox->OnValueChangedNoParams.RemoveDynamic(this, &UControlsSettingsWidget::OnApply);

	PrimaryKeyOptionBox->OnValueChangedNoParams.RemoveDynamic(this, &UControlsSettingsWidget::OnApply);
	SecondaryKeyOptionBox->OnValueChangedNoParams.RemoveDynamic(this, &UControlsSettingsWidget::OnApply);
	InteractKeyOptionBox->OnValueChangedNoParams.RemoveDynamic(this, &UControlsSettingsWidget::OnApply);
	ReloadKeyOptionBox->OnValueChangedNoParams.RemoveDynamic(this, &UControlsSettingsWidget::OnApply);

	InventoryKeyOptionBox->OnValueChangedNoParams.RemoveDynamic(this, &UControlsSettingsWidget::OnApply);
	CraftingKeyOptionBox->OnValueChangedNoParams.RemoveDynamic(this, &UControlsSettingsWidget::OnApply);
	ChatKeyOptionBox->OnValueChangedNoParams.RemoveDynamic(this, &UControlsSettingsWidget::OnApply);

	UWildOmissionGameUserSettings* UserSettings = UWildOmissionGameUserSettings::GetWildOmissionGameUserSettings();
	if (UserSettings == nullptr)
	{
		return;
	}

	const float MouseSensitivity = UserSettings->GetMouseSensitivity() * 10.0f;

	InvertMouseYCheckOptionBox->SetChecked(UserSettings->GetInvertedMouseY());
	MouseSensitivitySliderOptionBox->SetValue(MouseSensitivity);
	AlphaToolbarScrollCheckOptionBox->SetChecked(UserSettings->GetAlphaToolbarScroll());

	MoveForwardKeyOptionBox->SetSelectedKey(UserSettings->GetMoveForwardKey());
	MoveBackwardKeyOptionBox->SetSelectedKey(UserSettings->GetMoveBackwardKey());
	MoveLeftKeyOptionBox->SetSelectedKey(UserSettings->GetMoveLeftKey());
	MoveRightKeyOptionBox->SetSelectedKey(UserSettings->GetMoveRightKey());
	SprintKeyOptionBox->SetSelectedKey(UserSettings->GetSprintKey());
	CrouchKeyOptionBox->SetSelectedKey(UserSettings->GetCrouchKey());
	JumpKeyOptionBox->SetSelectedKey(UserSettings->GetJumpKey());
	FlyKeyOptionBox->SetSelectedKey(UserSettings->GetFlyKey());

	PrimaryKeyOptionBox->SetSelectedKey(UserSettings->GetPrimaryKey());
	SecondaryKeyOptionBox->SetSelectedKey(UserSettings->GetSecondaryKey());
	InteractKeyOptionBox->SetSelectedKey(UserSettings->GetInteractKey());
	ReloadKeyOptionBox->SetSelectedKey(UserSettings->GetReloadKey());

	InventoryKeyOptionBox->SetSelectedKey(UserSettings->GetInventoryKey());
	CraftingKeyOptionBox->SetSelectedKey(UserSettings->GetCraftingKey());
	ChatKeyOptionBox->SetSelectedKey(UserSettings->GetChatKey());

	InvertMouseYCheckOptionBox->OnValueChangedNoParams.AddDynamic(this, &UControlsSettingsWidget::OnApply);
	MouseSensitivitySliderOptionBox->OnValueChangedNoParams.AddDynamic(this, &UControlsSettingsWidget::OnApply);
	AlphaToolbarScrollCheckOptionBox->OnValueChangedNoParams.AddDynamic(this, &UControlsSettingsWidget::OnApply);

	MoveForwardKeyOptionBox->OnValueChangedNoParams.AddDynamic(this, &UControlsSettingsWidget::OnApply);
	MoveBackwardKeyOptionBox->OnValueChangedNoParams.AddDynamic(this, &UControlsSettingsWidget::OnApply);
	MoveLeftKeyOptionBox->OnValueChangedNoParams.AddDynamic(this, &UControlsSettingsWidget::OnApply);
	MoveRightKeyOptionBox->OnValueChangedNoParams.AddDynamic(this, &UControlsSettingsWidget::OnApply);
	SprintKeyOptionBox->OnValueChangedNoParams.AddDynamic(this, &UControlsSettingsWidget::OnApply);
	CrouchKeyOptionBox->OnValueChangedNoParams.AddDynamic(this, &UControlsSettingsWidget::OnApply);
	JumpKeyOptionBox->OnValueChangedNoParams.AddDynamic(this, &UControlsSettingsWidget::OnApply);
	FlyKeyOptionBox->OnValueChangedNoParams.AddDynamic(this, &UControlsSettingsWidget::OnApply);

	PrimaryKeyOptionBox->OnValueChangedNoParams.AddDynamic(this, &UControlsSettingsWidget::OnApply);
	SecondaryKeyOptionBox->OnValueChangedNoParams.AddDynamic(this, &UControlsSettingsWidget::OnApply);
	InteractKeyOptionBox->OnValueChangedNoParams.AddDynamic(this, &UControlsSettingsWidget::OnApply);
	ReloadKeyOptionBox->OnValueChangedNoParams.AddDynamic(this, &UControlsSettingsWidget::OnApply);

	InventoryKeyOptionBox->OnValueChangedNoParams.AddDynamic(this, &UControlsSettingsWidget::OnApply);
	CraftingKeyOptionBox->OnValueChangedNoParams.AddDynamic(this, &UControlsSettingsWidget::OnApply);
	ChatKeyOptionBox->OnValueChangedNoParams.AddDynamic(this, &UControlsSettingsWidget::OnApply);
}
