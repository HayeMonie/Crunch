// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_GroundBlast.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GAS/TargetActor_GroundPick.h"
#include "CAbilitySystemStatics.h"

UGA_GroundBlast::UGA_GroundBlast()
{
	ActivationOwnedTags.AddTag(UCAbilitySystemStatics::GetAimStatTag());
	BlockAbilitiesWithTag.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGA_GroundBlast::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                      const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Basic authority/prediction check
	if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		return;
	}

	// Play montage (if any)
	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, TargetingMontage);
	if (PlayMontageTask)
	{
		PlayMontageTask->OnBlendOut.AddDynamic(this, &UGA_GroundBlast::K2_EndAbility);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UGA_GroundBlast::K2_EndAbility);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UGA_GroundBlast::K2_EndAbility);
		PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_GroundBlast::K2_EndAbility);
		PlayMontageTask->ReadyForActivation();
	}

	// Wait for target data using a TargetActor class
	UAbilityTask_WaitTargetData* WaitTargetDataTask = UAbilityTask_WaitTargetData::WaitTargetData(this, NAME_None, EGameplayTargetingConfirmation::UserConfirmed, TargetActorClass);
	if (!WaitTargetDataTask)
	{
		K2_EndAbility();
		return;
	}

	WaitTargetDataTask->ValidData.AddDynamic(this, &UGA_GroundBlast::TargetConfirmed);
	WaitTargetDataTask->Cancelled.AddDynamic(this, &UGA_GroundBlast::TargetCancelled);

	// Spawn target actor if applicable
	AGameplayAbilityTargetActor* TargetActor = nullptr;
	if (WaitTargetDataTask->BeginSpawningActor(this, TargetActorClass, TargetActor))
	{
		ATargetActor_GroundPick* GroundPickActor = Cast<ATargetActor_GroundPick>(TargetActor);

		if (GroundPickActor)
		{
			GroundPickActor->SetShouldDrawDebug(ShouldDrawDebug());
			GroundPickActor->SetTargetAreaRadius(TargetAreaRadius);
			GroundPickActor->SetTargetTraceRange(TargetTraceRange);
		}
		
		// Optionally configure TargetActor here
		WaitTargetDataTask->FinishSpawningActor(this, TargetActor);
	}

	WaitTargetDataTask->ReadyForActivation();
}

void UGA_GroundBlast::TargetConfirmed(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}
	
	if (K2_HasAuthority())
	{
		BP_ApplyGameplayEffectToTarget(TargetDataHandle, DamageEffectDef.DamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		PushTargets(TargetDataHandle, DamageEffectDef.PushVelocity);	
	}


	FGameplayCueParameters BlastingGameplayCueParams;
	BlastingGameplayCueParams.Location = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(TargetDataHandle, 1).ImpactPoint;
	BlastingGameplayCueParams.RawMagnitude = TargetAreaRadius;

	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(BlastGameplayCueTag, BlastingGameplayCueParams);
	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(UCAbilitySystemStatics::GetCameraShakeGameplayCueTag(), BlastingGameplayCueParams);

	UAnimInstance* OwnerAnimInstance = GetOwnerAnimInstance();

	if (OwnerAnimInstance)
	{
		OwnerAnimInstance->Montage_Play(CastMontage);
	}
	
	K2_EndAbility();
}

void UGA_GroundBlast::TargetCancelled(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	UE_LOG(LogTemp, Warning, TEXT("Target Cancelled"))
	K2_EndAbility();
}
