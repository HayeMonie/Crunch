// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "TargetActor_Line.generated.h"

/**
 * 
 */
UCLASS()
class ATargetActor_Line : public AGameplayAbilityTargetActor, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ATargetActor_Line();
	void ConfigureTargetSetting(
		float NewTargetRange = -1.0f,
		float NewDetectionCylinderRadius = -1.0f,
		float NewTargetingInterval = -1.0f,
		FGenericTeamId OwnerTeamID = FGenericTeamId::NoTeam,
		bool bShouldDrawDebug = false
	);

	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

	FORCEINLINE virtual FGenericTeamId GetGenericTeamId() const override { return TeamID; };
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void StartTargeting(UGameplayAbility* Ability) override;
	virtual void Tick(float DeltaTime) override;
	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable, Category = "Lazer Debug")
	void SetDrawDebug(bool bDraw) { bDrawDebug = bDraw; }

	UFUNCTION(BlueprintPure, Category = "Lazer Debug")
	bool GetDrawDebug() const { return bDrawDebug; }

	// Capsule 属性暴露给蓝图
	UFUNCTION(BlueprintCallable, Category = "Lazer Config")
	void SetTargetRange(float NewRange) { TargetRange = NewRange; }

	UFUNCTION(BlueprintPure, Category = "Lazer Config")
	float GetTargetRange() const { return TargetRange; }

	UFUNCTION(BlueprintCallable, Category = "Lazer Config")
	void SetDetectionCylinderRadius(float NewRadius) { DetectionCylinderRadius = NewRadius; }

	UFUNCTION(BlueprintPure, Category = "Lazer Config")
	float GetDetectionCylinderRadius() const { return DetectionCylinderRadius; }

	UFUNCTION(BlueprintCallable, Category = "Lazer Config")
	void SetCollisionStartOffset(float NewOffset) { CollisionStartOffset = NewOffset; }

	UFUNCTION(BlueprintPure, Category = "Lazer Config")
	float GetCollisionStartOffset() const { return CollisionStartOffset; }

	UFUNCTION(BlueprintCallable, Category = "Lazer Config")
	void SetCollisionZOffset(float NewOffset) { CollisionZOffset = NewOffset; }

	UFUNCTION(BlueprintPure, Category = "Lazer Config")
	float GetCollisionZOffset() const { return CollisionZOffset; }

private:
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Lazer Config", Meta = (AllowPrivateAccess = "true"))
	float TargetRange;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Lazer Config", Meta = (AllowPrivateAccess = "true"))
	float DetectionCylinderRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lazer Config", Meta = (AllowPrivateAccess = "true"))
	float CollisionStartOffset { 0.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lazer Config", Meta = (AllowPrivateAccess = "true"))
	float CollisionZOffset { 0.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lazer Debug", Meta = (AllowPrivateAccess = "true"))
	bool bDrawDebug = false;

	UPROPERTY()
	float TargetingInterval;

	UPROPERTY(Replicated)
	FGenericTeamId TeamID;

	UPROPERTY(Replicated)
	const AActor* AvatarActor;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	FName LazerFXLengthParamName = TEXT("Length");
	
	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
	class USceneComponent* RootComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
	class UNiagaraComponent* LazerVFX;

	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
	class UCapsuleComponent* LazerCollisionCapsule;

	FTimerHandle PeriodicTargetingTimerHandle;

	void DoTargetCheckAndReport();

	void UpdateTargetTrace();

	bool ShouldReportActorAsTarget(const AActor* ActorToCheck) const;
};
