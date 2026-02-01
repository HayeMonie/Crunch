// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_Lazer.h"
#include "AbilitySystemComponent.h"
#include "CAttributeSet.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Abilities/Tasks/AbilityTask_WaitCancel.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "GAS/TargetActor_Line.h"

void UGA_Lazer::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility() || !LazerMontage)
	{
		K2_EndAbility();
		return;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayerLazerMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, LazerMontage);
		PlayerLazerMontageTask->OnBlendOut.AddDynamic(this, &UGA_Lazer::K2_EndAbility);
		PlayerLazerMontageTask->OnCancelled.AddDynamic(this, &UGA_Lazer::K2_EndAbility);
		PlayerLazerMontageTask->OnInterrupted.AddDynamic(this, &UGA_Lazer::K2_EndAbility);
		PlayerLazerMontageTask->OnCompleted.AddDynamic(this, &UGA_Lazer::K2_EndAbility);
		PlayerLazerMontageTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitShootEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetShootTag());
		WaitShootEvent->EventReceived.AddDynamic(this, &UGA_Lazer::ShootLazer);
		WaitShootEvent->ReadyForActivation();

		UAbilityTask_WaitCancel* WaitCancel = UAbilityTask_WaitCancel::WaitCancel(this);
		WaitCancel->OnCancel.AddDynamic(this, &UGA_Lazer::K2_EndAbility);
		WaitCancel->ReadyForActivation();

		UAbilityTask_WaitDelay* WaitDurationTask = UAbilityTask_WaitDelay::WaitDelay(this, MaxLazerDuration);
		WaitDurationTask->OnFinish.AddDynamic(this, &UGA_Lazer::K2_EndAbility);
		WaitDurationTask->ReadyForActivation();
	}
}

void UGA_Lazer::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UAbilitySystemComponent* OwnerAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (OwnerAbilitySystemComponent && OnGoingConsumptionEffectHandle.IsValid())
	{
		OwnerAbilitySystemComponent->RemoveActiveGameplayEffect(OnGoingConsumptionEffectHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FGameplayTag UGA_Lazer::GetShootTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Lazer.Shoot");
}

void UGA_Lazer::ShootLazer(FGameplayEventData Payload)
{
	if (K2_HasAuthority())
	{
		OnGoingConsumptionEffectHandle = BP_ApplyGameplayEffectToOwner(OnGoingConsumptionEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		UAbilitySystemComponent* OwnerAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
		if (OwnerAbilitySystemComponent)
		{
			OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetManaAttribute()).AddUObject(this, &UGA_Lazer::ManaUpdated);
		}
	}
	
	UAbilityTask_WaitTargetData* WaitDamageTargetTask = UAbilityTask_WaitTargetData::WaitTargetData(this, NAME_None, EGameplayTargetingConfirmation::CustomMulti, LazerTargetActorClass);
	WaitDamageTargetTask->ValidData.AddDynamic(this, &UGA_Lazer::TargetReceived);
	WaitDamageTargetTask->ReadyForActivation();

	AGameplayAbilityTargetActor* TargetActor;
	WaitDamageTargetTask->BeginSpawningActor(this, LazerTargetActorClass, TargetActor);

	ATargetActor_Line* LineTargetActor = Cast<ATargetActor_Line>(TargetActor);
	if (LineTargetActor)
	{
		// 传递 0.0f 表示不覆盖 TargetActor 蓝图中的设置
		// 如果想使用 GA_Lazer 中的设置，可以在 GA_Lazer 蓝图中修改这些值为正数
		LineTargetActor->ConfigureTargetSetting(0.0f, 0.0f, 0.0f, GetOwnerTeamId(), ShouldDrawDebug());
	}
	
	WaitDamageTargetTask->FinishSpawningActor(this, TargetActor);
	
	if (LineTargetActor)
	{
		LineTargetActor->AttachToComponent(GetOwningComponentFromActorInfo(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TargetActorAttachSocketName);
	}
}

void UGA_Lazer::ManaUpdated(const FOnAttributeChangeData& ChangeData)
{
	UAbilitySystemComponent* OwnerAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (OwnerAbilitySystemComponent && !OwnerAbilitySystemComponent->CanApplyAttributeModifiers(OnGoingConsumptionEffect.GetDefaultObject(),
		GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo), MakeEffectContext(CurrentSpecHandle, CurrentActorInfo)))
	{
		K2_EndAbility();
	}	
}

void UGA_Lazer::TargetReceived(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	// Debug信息已注释
	/*
	if (ShouldDrawDebug())
	{
		UE_LOG(LogTemp, Warning, TEXT("=== GA_Lazer TargetReceived ==="));
		UE_LOG(LogTemp, Warning, TEXT("TargetDataHandle.Num: %d"), TargetDataHandle.Num());

		for (int32 i = 0; i < TargetDataHandle.Num(); i++)
		{
			const FGameplayAbilityTargetData* TargetData = TargetDataHandle.Get(i);
			if (TargetData)
			{
				const TArray<TWeakObjectPtr<AActor>>& Actors = TargetData->GetActors();
				UE_LOG(LogTemp, Warning, TEXT("  Data[%d] has %d actors:"), i, Actors.Num());
				for (const TWeakObjectPtr<AActor>& WeakActor : Actors)
				{
					if (WeakActor.IsValid())
					{
						UE_LOG(LogTemp, Warning, TEXT("    - %s"), *WeakActor->GetName());
					}
				}
			}
		}
	}
	*/

	if (K2_HasAuthority())
	{
		/*
		if (ShouldDrawDebug())
		{
			UE_LOG(LogTemp, Warning, TEXT("Applying damage to targets..."));
		}
		*/
		BP_ApplyGameplayEffectToTarget(TargetDataHandle, HitDamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
	}
	/*
	else
	{
		if (ShouldDrawDebug())
		{
			UE_LOG(LogTemp, Warning, TEXT("Not on server, skipping damage application"));
		}
	}
	*/

	/*
	if (ShouldDrawDebug())
	{
		UE_LOG(LogTemp, Warning, TEXT("Applying knockback to targets..."));
	}
	*/
	PushTargets(TargetDataHandle, GetAvatarActorFromActorInfo()->GetActorForwardVector() * HitPushSpeed);
}
