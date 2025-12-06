// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/OverHeadStatsGauge.h"
#include "Widgets/ValueGauge.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/CAttributeSet.h"

void UOverHeadStatsGauge::ConfigureWithASC(class UAbilitySystemComponent* AbilitySystemComponent)
{
	// AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());

	if (AbilitySystemComponent)
	{
		HealthBar->SetAndBoundToGameplayAttribute(AbilitySystemComponent, UCAttributeSet::GetHealthAttribute(), UCAttributeSet::GetMaxHealthAttribute());
		ManaBar->SetAndBoundToGameplayAttribute(AbilitySystemComponent, UCAttributeSet::GetManaAttribute(), UCAttributeSet::GetMaxManaAttribute());
	}
}
