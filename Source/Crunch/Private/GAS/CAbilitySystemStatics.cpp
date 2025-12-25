// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CAbilitySystemStatics.h"

#include "CAttributeSet.h"
#include "Abilities/GameplayAbility.h"

FGameplayTag UCAbilitySystemStatics::GetBasicAttackAbilityTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.BasicAttack");
}

FGameplayTag UCAbilitySystemStatics::GetBasicAttackInputPressedTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.BasicAttack.Pressed");
}

FGameplayTag UCAbilitySystemStatics::GetDeadStatTag()
{
	return FGameplayTag::RequestGameplayTag("Stats.Dead");
}

FGameplayTag UCAbilitySystemStatics::GetStunStatTag()
{
	return FGameplayTag::RequestGameplayTag("Stats.Stun");
}

FGameplayTag UCAbilitySystemStatics::GetAimStatTag()
{
	return FGameplayTag::RequestGameplayTag("Stats.Aim");
}

FGameplayTag UCAbilitySystemStatics::GetHealthFullStatTag()
{
	return FGameplayTag::RequestGameplayTag("Stats.Health.Full");
}

FGameplayTag UCAbilitySystemStatics::GetHealthEmptyStatTag()
{
	return FGameplayTag::RequestGameplayTag("Stats.Health.Empty");
}

FGameplayTag UCAbilitySystemStatics::GetManaFullStatTag()
{
	return FGameplayTag::RequestGameplayTag("Stats.Mana.Full");
}

FGameplayTag UCAbilitySystemStatics::GetManaEmptyStatTag()
{
	return FGameplayTag::RequestGameplayTag("Stats.Mana.Empty");
}

FGameplayTag UCAbilitySystemStatics::GetCameraShakeGameplayCueTag()
{
	return FGameplayTag::RequestGameplayTag("GameplayCue.CameraShake");
}

float UCAbilitySystemStatics::GetStaticCooldownDurationForAbility(const UGameplayAbility* Ability)
{
	if (!Ability)
	{
		return 0.f;
	}

	const UGameplayEffect* CooldownEffect = Ability->GetCooldownGameplayEffect();
	if (!CooldownEffect)
	{
		return 0.f;
	}

	float CooldownDuration = 0.f;

	CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(1, CooldownDuration);

	return CooldownDuration;
}

float UCAbilitySystemStatics::GetStaticCostForAbility(const UGameplayAbility* Ability)
{
	if (!Ability)
	{
		return 0.f;
	}
	
	const UGameplayEffect* CostEffect = Ability->GetCostGameplayEffect();
	if (!CostEffect || CostEffect->Modifiers.Num() == 0)
	{
		return 0.f;
	}

	float Cost = 0.f;

	CostEffect->Modifiers[0].ModifierMagnitude.GetStaticMagnitudeIfPossible(1, Cost);

	return FMath::Abs(Cost);
}
