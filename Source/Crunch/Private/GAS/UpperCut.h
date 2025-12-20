// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/CGameplayAbility.h"
#include "UpperCut.generated.h"

/**
 * 
 */
UCLASS()
class UUpperCut : public UCGameplayAbility
{
	GENERATED_BODY()
	
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	UUpperCut();

protected:
	// 检测相关属性（放在 protected，允许 Blueprint 读写）
	// TargetSweepSphereRadius:
	// - 单位：厘米（与 UE 中常用单位一致）
	// - 语义：用于每次采样时球形 sweep 的半径（即胶囊圆柱段的半径）
	// - 影响：增大此值会扩大水平/径向覆盖范围，但会影响命中判定精度与性能（更大半径 -> 更大采样覆盖）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting", meta = (ToolTip = "检测球半径（厘米）。增大可扩大水平覆盖范围）"))
	float TargetSweepSphereRadius = 80.f;
	
	// TargetSweepCapsuleHalfHeight:
	// - 单位：厘米（半高度）
	// - 语义：用于描述胶囊几何中圆柱部分的一半高度（两端半球之间半距离）
	// - 影响：增大该值会“拉长”胶囊的圆柱段（通过采样多个球近似实现），值为 0 时等同于单个球形 sweep
	// - 注意：本设计通过多次球形 sweep 采样近似胶囊，因此半径与半高度共同决定采样点数与覆盖
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting", meta = (ToolTip = "胶囊半高度（厘米）。增大可拉长圆柱段长度）"))
	float TargetSweepCapsuleHalfHeight = 120.f;
	
	// TargetSweepZOffset:
	// - 单位：厘米
	// - 语义：在角色位置基础上沿世界 Up 轴的偏移，用于把检测中心抬高或降低
	// - 影响：改变检测整体在垂直方向的位置（例如上勾拳需要把检测中心抬高到空中目标）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting", meta = (ToolTip = "垂直偏移（厘米）。增大该值会把检测中心向上抬高，从而改变检测高度）"))
	float TargetSweepZOffset = 50.f;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	TSubclassOf<UGameplayEffect> LaunchDamageEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	TSubclassOf<UGameplayEffect> ComboDamageEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	float UpperCutLaunchSpeed = 1400.f;

	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	float UpperComboHoldSpeed = 160.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* UpperCutMontage;

	static FGameplayTag GetUpperCutLaunchTag();

	UFUNCTION()
	void StartLaunching(FGameplayEventData EventData);

	UFUNCTION()
	void HandleComboChangeEvent(FGameplayEventData EventData);

	UFUNCTION()
	void HandleComboCommitEvent(FGameplayEventData EventData);

	UFUNCTION()
	void HandleComboDamageEvent(FGameplayEventData EventData);

	FName NextComboName;

	// 新增方法声明的注释：
	// GetHitResultsFromCapsuleTargetData:
	// - 功能：使用真实胶囊（Capsule）在世界中做 Overlap 来检测命中目标（替换之前的多球采样近似方法）。
	// - 实现要点：
	//   * 使用 Avatar->GetWorld()->OverlapMultiByChannel + FCollisionShape::MakeCapsule。
	//   * 以 Avatar 的位置 + LocationOffset 为胶囊中心，半径为 SphereSweepRadius，半高度为 CapsuleHalfHeight。
	//   * 根据 bIgnoreSelf 决定是否把 Avatar 自身加入忽略列表。
	//   * 按 Actor 去重并把每个重叠项转换为 FHitResult 返回，供上层 ApplyGameplayEffect 等使用。
	//   * 当 bDrewDebug 为 true 时，在世界中绘制单个胶囊线框（避免之前多球采样导致的多重 debug）。
	TArray<FHitResult> GetHitResultsFromCapsuleTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle, float SphereSweepRadius, float CapsuleHalfHeight, ETeamAttitude::Type TargetTeam, bool bDrewDebug, bool bIgnoreSelf, const FVector& LocationOffset) const;
};
