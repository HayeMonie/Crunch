// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CAbilitySystemStatics.h"

FGameplayTag UCAbilitySystemStatics::GetBasicAttackAbilityTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.BasicAttack");
}

FGameplayTag UCAbilitySystemStatics::GetBasicAttackInputPressedTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.BasicAttack.Pressed");
}

FGameplayTag UCAbilitySystemStatics::GetDeadStatsTag()
{
	return FGameplayTag::RequestGameplayTag("Stats.Dead");
}

FGameplayTag UCAbilitySystemStatics::GetStunStatsTag()
{
	return FGameplayTag::RequestGameplayTag("Stats.Stun");
}
