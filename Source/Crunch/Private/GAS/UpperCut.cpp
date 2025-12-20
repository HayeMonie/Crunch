// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/UpperCut.h"
#include "GameplayTagsManager.h"
#include "GA_Combo.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

UUpperCut::UUpperCut()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	BlockAbilitiesWithTag.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UUpperCut::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (!UpperCutMontage)
		{
			UE_LOG(LogTemp, Error, TEXT("UpperCutMontage is not set!"));
			K2_EndAbility();
			return;
		}

		UAbilityTask_PlayMontageAndWait* PlayUpperCutMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, UpperCutMontage);
		PlayUpperCutMontageTask->OnBlendOut.AddDynamic(this, &UUpperCut::K2_EndAbility);
		PlayUpperCutMontageTask->OnCancelled.AddDynamic(this, &UUpperCut::K2_EndAbility);
		PlayUpperCutMontageTask->OnInterrupted.AddDynamic(this, &UUpperCut::K2_EndAbility);
		PlayUpperCutMontageTask->OnCompleted.AddDynamic(this, &UUpperCut::K2_EndAbility);
		PlayUpperCutMontageTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitLaunchEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetUpperCutLaunchTag());
		WaitLaunchEventTask->EventReceived.AddDynamic(this, &UUpperCut::StartLaunching);
		WaitLaunchEventTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitComboChangeEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UGA_Combo::GetComboChangedEventTag(), nullptr, false, false);
		WaitComboChangeEvent->EventReceived.AddDynamic(this, &UUpperCut::HandleComboChangeEvent);
		WaitComboChangeEvent->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitComboCommitEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UCAbilitySystemStatics::GetBasicAttackInputPressedTag());
		WaitComboCommitEvent->EventReceived.AddDynamic(this, &UUpperCut::HandleComboCommitEvent);
		WaitComboCommitEvent->ReadyForActivation();
	}

	if (K2_HasAuthority())
	{
		UAbilityTask_WaitGameplayEvent* WaitComboDamageEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UGA_Combo::GetComboTargetEventTag());
		WaitComboDamageEvent->EventReceived.AddDynamic(this, &UUpperCut::HandleComboDamageEvent);
		WaitComboDamageEvent->ReadyForActivation();
	}
	NextComboName = NAME_None;
	
}



FGameplayTag UUpperCut::GetUpperCutLaunchTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.UpperCut.Launch");
}

void UUpperCut::StartLaunching(FGameplayEventData EventData)
{

	if (K2_HasAuthority())
	{
		TArray<FHitResult> TargetHitResults = GetHitResultFromSweepLocationTargetData(EventData.TargetData, TargetSweepSphereRadius, ETeamAttitude::Hostile, ShouldDrawDebug());

		PushTarget(GetAvatarActorFromActorInfo(), FVector::UpVector * UpperCutLaunchSpeed);
		
		for (FHitResult& HitResult : TargetHitResults)
		{
			PushTarget(HitResult.GetActor(), FVector::UpVector * UpperCutLaunchSpeed);				
			ApplyGameplayEffectToHitResultActor(HitResult, LaunchDamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		}
	}
}

void UUpperCut::HandleComboChangeEvent(FGameplayEventData EventData)
{
	FGameplayTag EventTag = EventData.EventTag;

	if (EventTag == UGA_Combo::GetComboChangedEventEndTag())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ChangeEvent] Received END event"));
		NextComboName = NAME_None;
		
		// 如果当前在 combo02，结束 Ability
		UAnimInstance* OwnerAnimInst = GetOwnerAnimInstance();
		if (OwnerAnimInst)
		{
			FName CurrentSection = OwnerAnimInst->Montage_GetCurrentSection(UpperCutMontage);
			UE_LOG(LogTemp, Warning, TEXT("[ChangeEvent] Current Section: %s"), *CurrentSection.ToString());
			if (CurrentSection == FName("combo02"))
			{
				UE_LOG(LogTemp, Warning, TEXT("[ChangeEvent] Ending Ability"));
				K2_EndAbility();
			}
		}
		return;
	}

	TArray<FName> TagNames;
	UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);

	NextComboName = TagNames.Last();
	UE_LOG(LogTemp, Warning, TEXT("[ChangeEvent] NextCombo set to: %s"), *NextComboName.ToString());
}

void UUpperCut::HandleComboCommitEvent(FGameplayEventData EventData)
{
	if (NextComboName == NAME_None)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CommitEvent] NextComboName is None"));
		return;
	}

	UAnimInstance* OwnerAnimInst = GetOwnerAnimInstance();
	if (!OwnerAnimInst)
	{
		return;
	}

	FName CurrentSection = OwnerAnimInst->Montage_GetCurrentSection(UpperCutMontage);
	UE_LOG(LogTemp, Warning, TEXT("[CommitEvent] SetNextSection: %s -> %s"), *CurrentSection.ToString(), *NextComboName.ToString());
	
	OwnerAnimInst->Montage_SetNextSection(OwnerAnimInst->Montage_GetCurrentSection(UpperCutMontage), NextComboName, UpperCutMontage);
	NextComboName = NAME_None;
}

void UUpperCut::HandleComboDamageEvent(FGameplayEventData EventData)
{
	if (K2_HasAuthority())
	{
		TArray<FHitResult> TargetHitResults = GetHitResultFromSweepLocationTargetData(EventData.TargetData, TargetSweepSphereRadius, ETeamAttitude::Hostile, ShouldDrawDebug());
		PushTarget(GetAvatarActorFromActorInfo(), FVector::UpVector * UpperComboHoldSpeed);	
		for (FHitResult& HitResult : TargetHitResults)
		{
			ApplyGameplayEffectToHitResultActor(HitResult, ComboDamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		}
	}
}
