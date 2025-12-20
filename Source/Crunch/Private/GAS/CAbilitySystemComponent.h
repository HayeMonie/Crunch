// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "CAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class CRUNCH_API UCAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UCAbilitySystemComponent();
	void ApplyInitialEffects();
	void GiveInitialAbilities();
	void ApplyFullStatEffect();
	
	// Get the Abilities that is unique to this character, this DO NOT include generic or basic abilities
	const TMap<ECAbilityInputID, TSubclassOf<UGameplayAbility>>& GetAbilities() const;
	
private:
	void AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int Level = 1); 
	void HealthUpdated(const FOnAttributeChangeData& ChangeData);

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TSubclassOf<UGameplayEffect> FullStatEffect; 

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TSubclassOf<UGameplayEffect> DeathEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TArray<TSubclassOf<UGameplayEffect>> InitialEffects;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	TMap<ECAbilityInputID, TSubclassOf<UGameplayAbility>> Abilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	TMap<ECAbilityInputID, TSubclassOf<UGameplayAbility>> BasicAbilities;

	
};
