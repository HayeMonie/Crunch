// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Abilities/GameplayAbility.h"
#include "CGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class UCGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UCGameplayAbility();

	
protected:
	class UAnimInstance* GetOwnerAnimInstance() const;
	TArray<FHitResult> GetHitResultFromSweepLocationTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle,
		float SphereSweepRadius = 30.f, ETeamAttitude::Type TargetTeam = ETeamAttitude::Hostile, bool bDrewDebug = false, bool bIgnoreSelf = true) const;

	UFUNCTION()
	FORCEINLINE bool ShouldDrawDebug() const { return bShouldDrawDebug; }
	void PushSelf(const FVector& PushVelocity);

	void PushTarget(AActor* Target, const FVector& PushVelocity);

	ACharacter* GetOwningAvatarCharacter();
	void ApplyGameplayEffectToHitResultActor(const FHitResult& HitResult, TSubclassOf<UGameplayEffect> GameplayEffect, int Level = 1);
	
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bShouldDrawDebug = false;

	UPROPERTY()
	class ACharacter* AvatarCharacter;
};
