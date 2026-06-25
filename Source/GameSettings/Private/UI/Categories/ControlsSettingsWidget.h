// Copyright Telephone Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SettingsCategoryWidget.h"
#include "ControlsSettingsWidget.generated.h"

UCLASS()
class UControlsSettingsWidget : public USettingsCategoryWidget
{
	GENERATED_BODY()

public:
	UControlsSettingsWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void OnApply() override;
	virtual void OnRefresh() override;

private:
	UPROPERTY(Meta = (BindWidget))
	class UCheckOptionBox* InvertMouseYCheckOptionBox;
	UPROPERTY(Meta = (BindWidget))
	class UCheckOptionBox* AlphaToolbarScrollCheckOptionBox;
	UPROPERTY(Meta = (BindWidget))
	class USliderOptionBox* MouseSensitivitySliderOptionBox;
	UPROPERTY(Meta = (BindWidget))
	class UKeyOptionBox* MoveForwardKeyOptionBox;
	UPROPERTY(Meta = (BindWidget))
	class UKeyOptionBox* MoveBackwardKeyOptionBox;
	UPROPERTY(Meta = (BindWidget))
	class UKeyOptionBox* MoveLeftKeyOptionBox;
	UPROPERTY(Meta = (BindWidget))
	class UKeyOptionBox* MoveRightKeyOptionBox;
	UPROPERTY(Meta = (BindWidget))
	class UKeyOptionBox* SprintKeyOptionBox;
	UPROPERTY(Meta = (BindWidget))
	class UKeyOptionBox* CrouchKeyOptionBox;
	UPROPERTY(Meta = (BindWidget))
	class UKeyOptionBox* JumpKeyOptionBox;
	UPROPERTY(Meta = (BindWidget))
	class UKeyOptionBox* FlyKeyOptionBox;

	UPROPERTY(Meta = (BindWidget))
	class UKeyOptionBox* PrimaryKeyOptionBox;
	UPROPERTY(Meta = (BindWidget))
	class UKeyOptionBox* SecondaryKeyOptionBox;
	UPROPERTY(Meta = (BindWidget))
	class UKeyOptionBox* InteractKeyOptionBox;
	UPROPERTY(Meta = (BindWidget))
	class UKeyOptionBox* GrabKeyOptionBox;
	UPROPERTY(Meta = (BindWidget))
	class UKeyOptionBox* ReloadKeyOptionBox;

	UPROPERTY(Meta = (BindWidget))
	class UKeyOptionBox* InventoryKeyOptionBox;
	UPROPERTY(Meta = (BindWidget))
	class UKeyOptionBox* CraftingKeyOptionBox;
	UPROPERTY(Meta = (BindWidget))
	class UKeyOptionBox* ChatKeyOptionBox;


};
