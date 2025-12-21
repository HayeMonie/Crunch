// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/AbilityGauge.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/Image.h"
#include "GAS/CAbilitySystemStatics.h"
#include "Components/TextBlock.h"

void UAbilityGauge::NativeConstruct()
{
	Super::NativeConstruct();

	CooldownCounterText->SetVisibility(ESlateVisibility::Hidden);
	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
	if (OwnerASC)
	{
		OwnerASC->AbilityCommittedCallbacks.AddUObject(this, &UAbilityGauge::AbilityCommitted);
	}

	WholeNumberFormattingOptions.MaximumFractionalDigits = 0;
	TwoDigitNumberFormattingOptions.MaximumFractionalDigits = 2;
}

void UAbilityGauge::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	AbilityCDO = Cast<UGameplayAbility>(ListItemObject);

	float CooldownDuration = UCAbilitySystemStatics::GetStaticCooldownDurationForAbility(AbilityCDO);
	float Cost = UCAbilitySystemStatics::GetStaticCostForAbility(AbilityCDO);

	CooldownDurationText->SetText(FText::AsNumber(CooldownDuration));
	CostText->SetText(FText::AsNumber(Cost));
	
}

void UAbilityGauge::ConfigureWithWidgetData(const FAbilityWidgetData* WidgetData)
{
	if (Icon && WidgetData)
	{
		UTexture2D* IconTexture = WidgetData->Icon.LoadSynchronous();
		if (IconTexture)
		{
			// 尝试获取或创建动态材质实例
			UMaterialInstanceDynamic* DynamicMaterial = Icon->GetDynamicMaterial();
			if (!DynamicMaterial)
			{
				// 如果没有动态材质,直接设置纹理
				Icon->SetBrushFromTexture(IconTexture);
			}
			else
			{
				// 如果有动态材质,通过参数设置
				DynamicMaterial->SetTextureParameterValue(IconMaterialParameterName, IconTexture);
			}
		}
	}
}

void UAbilityGauge::AbilityCommitted(UGameplayAbility* Ability)
{
	if (Ability->GetClass()->GetDefaultObject() == AbilityCDO)
	{
		float CooldownTimeRemaining = 0.f;
		float CooldownDuration = 0.f;

		Ability->GetCooldownTimeRemainingAndDuration(Ability->GetCurrentAbilitySpecHandle(), Ability->GetCurrentActorInfo(), CooldownTimeRemaining, CooldownDuration);

		StartCooldown(CooldownTimeRemaining, CooldownDuration);
	}
}

void UAbilityGauge::StartCooldown(float CooldownTimeRemaining, float CooldownDuration)
{
	CachedCooldownDuration = CooldownDuration;
	CooldownDurationText->SetText(FText::AsNumber(CooldownDuration));

	CachedCooldownTimeRemaining = CooldownTimeRemaining;

	CooldownCounterText->SetVisibility(ESlateVisibility::Visible);
	GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &UAbilityGauge::CooldownFinished, CooldownTimeRemaining);
	GetWorld()->GetTimerManager().SetTimer(CooldownTimerUpdateHandle, this, &UAbilityGauge::UpdateCooldown, CooldownUpdateInterval, true, 0.f);
}

void UAbilityGauge::CooldownFinished()
{
	CachedCooldownDuration = CachedCooldownTimeRemaining = 0.f;
	CooldownCounterText->SetVisibility(ESlateVisibility::Hidden);
	GetWorld()->GetTimerManager().ClearTimer(CooldownTimerUpdateHandle);
	
	if (Icon)
	{
		UMaterialInstanceDynamic* DynamicMaterial = Icon->GetDynamicMaterial();
		if (DynamicMaterial)
		{
			DynamicMaterial->SetScalarParameterValue(CooldownPercentParamName, 1.f);
		}
	}
}

void UAbilityGauge::UpdateCooldown()
{
	CachedCooldownTimeRemaining -= CooldownUpdateInterval;

	// 根据选项决定是否显示小数部分
	FNumberFormattingOptions* FormattingOptions = nullptr;
	if (CachedCooldownTimeRemaining > 1.0f && !bShowDecimalsWhenAboveOneSecond)
	{
		// 大于1秒且不显示小数,使用整数格式
		FormattingOptions = &WholeNumberFormattingOptions;
	}
	else
	{
		// 小于等于1秒或需要显示小数,使用2位小数格式
		FormattingOptions = &TwoDigitNumberFormattingOptions;
	}
	
	CooldownCounterText->SetText(FText::AsNumber(CachedCooldownTimeRemaining, FormattingOptions));

	if (Icon)
	{
		UMaterialInstanceDynamic* DynamicMaterial = Icon->GetDynamicMaterial();
		if (DynamicMaterial)
		{
			DynamicMaterial->SetScalarParameterValue(CooldownPercentParamName, 1.f - CachedCooldownTimeRemaining / CachedCooldownDuration);
		}
	}
}
