// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/CGameplayAbility.h"
#include "TargetActor_GroundPick.h"
#include "GA_BlackHole.generated.h"

/**
 * 
 */
UCLASS()
class UGA_BlackHole : public UCGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float TargetAreaRadius {1000.f};

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float BlackHolePullSpeed {300.f};

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float BlackHoleDuration {6.f};

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float TargetTraceRange {2000.f};

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	TSubclassOf<UGameplayEffect> AimEffect;

	FActiveGameplayEffectHandle AimEffectHandle;
	
	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	TSubclassOf<class ATargetActor_GroundPick> TargetActorClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	TSubclassOf<class ATA_BlackHole> BlackHoleTargetActorClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TSubclassOf<class UGameplayEffect> FinalBlowDamageEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	float BlowPushSpeed {3000.f};
	
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* TargetingMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* HoldBlackHoleMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* FinalBlowMontage;

	UPROPERTY()
	class UAbilityTask_PlayMontageAndWait* PlayCastBlackHoleMontageTask;

	UPROPERTY()
	class UAbilityTask_WaitTargetData* BlackHoleTargetingTask;

	UFUNCTION()
	void PlaceBlackHole(const FGameplayAbilityTargetDataHandle& TargetDataHandle);

	UFUNCTION()
	void PlacementCancelled(const FGameplayAbilityTargetDataHandle& TargetDataHandle);

	UFUNCTION()
	void FinalTargetsReceived(const FGameplayAbilityTargetDataHandle& TargetDataHandle);
	
	void AddAimEffect();
	void RemoveAimEffect();
};
