// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GAP_Dead.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CAbilitySystemStatics.h"
#include "CHeroAttributeSet.h"
#include "Engine/OverlapResult.h"

UGAP_Dead::UGAP_Dead()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	TriggerData.TriggerTag = UCAbilitySystemStatics::GetDeadStatTag();
	
	AbilityTriggers.Add(TriggerData);
	ActivationBlockedTags.RemoveTag(UCAbilitySystemStatics::GetStunStatTag());
}

void UGAP_Dead::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (K2_HasAuthority())
	{
		AActor* Killer = TriggerEventData->ContextHandle.GetEffectCauser();
		if (!Killer || !UCAbilitySystemStatics::IsHero(Killer))
		{
			Killer = nullptr;
		}

		TArray<AActor*> RewardTargets = GetRewardTargets();
		if (RewardTargets.Num() == 0 && !Killer)
		{
			K2_EndAbility();
			return;
		}

		if (Killer && !RewardTargets.Contains(Killer))
		{
			RewardTargets.Add(Killer);
		}

		// 获取被击杀单位的当前经验值，用于计算动态奖励
		// bFound用于标记属性是否成功获取
		bool bFound = false;
		// 从被击杀单位的ASC中读取Experience属性值
		// 该值会与ExperienceRewardPerExperience系数相乘，形成额外的经验奖励
		// 例如：被击杀单位经验越高，击杀者获得的奖励也越多（类似MOBA中击杀高等级单位获得更多奖励）
		float SelfExperience = GetAbilitySystemComponentFromActorInfo_Ensured()->GetGameplayAttributeValue(UCHeroAttributeSet::GetExperienceAttribute(), bFound);
		
		float TotalExperienceReward = BaseExperienceReward + ExperienceRewardPerExperience * SelfExperience;
		float TotalGoldReward = BaseGoldReward + GoldRewardPerExperience * SelfExperience;

		if (Killer)
		{
			float KillerExperienceReward = TotalExperienceReward * KillerRewardPortion;
			float KillerGoldReward = TotalGoldReward * KillerRewardPortion;

			FGameplayEffectSpecHandle EffectSpec = MakeOutgoingGameplayEffectSpec(RewardEffect);
			EffectSpec.Data->SetSetByCallerMagnitude(UCAbilitySystemStatics::GetExperienceAttributeTag(), KillerExperienceReward);
			EffectSpec.Data->SetSetByCallerMagnitude(UCAbilitySystemStatics::GetGoldAttributeTag(), KillerGoldReward);

			K2_ApplyGameplayEffectSpecToTarget(EffectSpec, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(Killer));

			TotalExperienceReward -= KillerExperienceReward;
			TotalGoldReward -= KillerGoldReward;
		}

		// Killer算在分母中，但从实际分配列表中移除，确保队友获得正确的人均奖励
		int32 TotalRewardCount = RewardTargets.Num();
		RewardTargets.Remove(Killer);

		if (RewardTargets.Num() > 0)
		{
			float ExperiencePerTarget = TotalExperienceReward  / TotalRewardCount;
			float GoldPerTarget = TotalGoldReward / TotalRewardCount;

			FGameplayEffectSpecHandle EffectSpec = MakeOutgoingGameplayEffectSpec(RewardEffect);
			EffectSpec.Data->SetSetByCallerMagnitude(UCAbilitySystemStatics::GetExperienceAttributeTag(), ExperiencePerTarget);
			EffectSpec.Data->SetSetByCallerMagnitude(UCAbilitySystemStatics::GetGoldAttributeTag(), GoldPerTarget);

			K2_ApplyGameplayEffectSpecToTarget(EffectSpec, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActorArray(RewardTargets, true));
		}
		K2_EndAbility();
	}
}

TArray<AActor*> UGAP_Dead::GetRewardTargets() const
{
	TSet<AActor*> OutActors;

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !GetWorld())
	{
		return OutActors.Array();
	}

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	FCollisionShape  CollisionShape;
	CollisionShape.SetSphere(RewardRange);

	TArray<FOverlapResult> OverlapResults;
	if (GetWorld()->OverlapMultiByObjectType(OverlapResults, AvatarActor->GetActorLocation(), FQuat::Identity, ObjectQueryParams, CollisionShape))
	{
		for (const FOverlapResult& OverlapResult : OverlapResults)
		{
			const IGenericTeamAgentInterface* OtherTeamInterface = Cast<IGenericTeamAgentInterface>(OverlapResult.GetActor());
			if (!OtherTeamInterface || OtherTeamInterface->GetTeamAttitudeTowards(*AvatarActor) != ETeamAttitude::Hostile)
			{
				continue;
			}

			if (!UCAbilitySystemStatics::IsHero(OverlapResult.GetActor()))
			{
				continue;
			}

			OutActors.Add(OverlapResult.GetActor());
		}
	}

	return OutActors.Array();
}
