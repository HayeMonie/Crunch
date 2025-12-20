// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/UpperCut.h"
#include "GameplayTagsManager.h"
#include "GA_Combo.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "Engine/EngineTypes.h"

// UUpperCut 类的构造函数
UUpperCut::UUpperCut()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor; // 设置实例化策略
	BlockAbilitiesWithTag.AddTag(UCAbilitySystemStatics::GetBasicAttackAbilityTag()); // 阻止带有基本攻击标签的能力
	UpperCutMontage = nullptr; // 初始化 UpperCutMontage 为 nullptr
}

// 激活能力的函数
void UUpperCut::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData); // 调用父类的激活能力函数

	if (!K2_CommitAbility()) // 提交能力，如果失败则结束能力
	{
		K2_EndAbility();
		return;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo)) // 检查是否具有权限或预测密钥
	{
		if (!UpperCutMontage) // 如果 UpperCutMontage 未设置，则输出错误日志并结束能力
		{
			UE_LOG(LogTemp, Error, TEXT("UpperCutMontage is not set!"));
			K2_EndAbility();
			return;
		}

		// 播放 UpperCut 动画蒙太奇
		UAbilityTask_PlayMontageAndWait* PlayUpperCutMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, UpperCutMontage);
		PlayUpperCutMontageTask->OnBlendOut.AddDynamic(this, &UUpperCut::K2_EndAbility);
		PlayUpperCutMontageTask->OnCancelled.AddDynamic(this, &UUpperCut::K2_EndAbility);
		PlayUpperCutMontageTask->OnInterrupted.AddDynamic(this, &UUpperCut::K2_EndAbility);
		PlayUpperCutMontageTask->OnCompleted.AddDynamic(this, &UUpperCut::K2_EndAbility);
		PlayUpperCutMontageTask->ReadyForActivation();

		// 等待上升事件
		UAbilityTask_WaitGameplayEvent* WaitLaunchEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetUpperCutLaunchTag());
		WaitLaunchEventTask->EventReceived.AddDynamic(this, &UUpperCut::StartLaunching);
		WaitLaunchEventTask->ReadyForActivation();

		// 等待连击变化事件
		UAbilityTask_WaitGameplayEvent* WaitComboChangeEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UGA_Combo::GetComboChangedEventTag(), nullptr, false, false);
		WaitComboChangeEvent->EventReceived.AddDynamic(this, &UUpperCut::HandleComboChangeEvent);
		WaitComboChangeEvent->ReadyForActivation();

		// 等待连击确认事件
		UAbilityTask_WaitGameplayEvent* WaitComboCommitEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, UCAbilitySystemStatics::GetBasicAttackInputPressedTag());
		WaitComboCommitEvent->EventReceived.AddDynamic(this, &UUpperCut::HandleComboCommitEvent);
		WaitComboCommitEvent->ReadyForActivation();
	}

	if (K2_HasAuthority()) // 如果具有权限，则等待连击伤害事件
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

// 开始上升的函数
void UUpperCut::StartLaunching(FGameplayEventData EventData)
{

	if (K2_HasAuthority())
	{
		AActor* Avatar = GetAvatarActorFromActorInfo();
		if (!Avatar) return;
		FVector LocationOffset = FVector::UpVector * TargetSweepZOffset;

		TArray<FHitResult> TargetHitResults = GetHitResultFromSweepLocationTargetData(EventData.TargetData, TargetSweepSphereRadius, ETeamAttitude::Hostile, ShouldDrawDebug(), true, LocationOffset);

		PushTarget(GetAvatarActorFromActorInfo(), FVector::UpVector * UpperCutLaunchSpeed);
		
		for (FHitResult& HitResult : TargetHitResults)
		{
			PushTarget(HitResult.GetActor(), FVector::UpVector * UpperCutLaunchSpeed);                
			ApplyGameplayEffectToHitResultActor(HitResult, LaunchDamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		}
	}
}

// 处理连击变化事件的函数
void UUpperCut::HandleComboChangeEvent(FGameplayEventData EventData)
{
	FGameplayTag EventTag = EventData.EventTag;

	if (EventTag == UGA_Combo::GetComboChangedEventEndTag()) // 如果是连击结束事件
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

// 处理连击确认事件的函数
void UUpperCut::HandleComboCommitEvent(FGameplayEventData EventData)
{
	if (NextComboName == NAME_None) // 如果下一个连击名称为空
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

// 处理连击伤害事件的函数
void UUpperCut::HandleComboDamageEvent(FGameplayEventData EventData)
{
	if (K2_HasAuthority())
	{
		AActor* Avatar = GetAvatarActorFromActorInfo();
		if (!Avatar) return;
		FVector LocationOffset = FVector::UpVector * TargetSweepZOffset;

		TArray<FHitResult> TargetHitResults = GetHitResultFromSweepLocationTargetData(EventData.TargetData, TargetSweepSphereRadius, ETeamAttitude::Hostile, ShouldDrawDebug(), true, LocationOffset);
		PushTarget(GetAvatarActorFromActorInfo(), FVector::UpVector * UpperComboHoldSpeed);    
		for (FHitResult& HitResult : TargetHitResults)
		{
			ApplyGameplayEffectToHitResultActor(HitResult, ComboDamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		}
	}
}
