// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "Components/SphereComponent.h"
#include "GenericTeamAgentInterface.h"
#include "TA_BlackHole.generated.h"

/**
 * 
 */
UCLASS()
class ATA_BlackHole : public AGameplayAbilityTargetActor, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ATA_BlackHole();
	void ConfigureBlackHole(float InBlackHoleRange, float InPullSpeed, float BlackHoleDuration, const FGenericTeamId& InTeamId);

	virtual void SetGenericTeamId(const FGenericTeamId& InTeamId) override;
	
	virtual FGenericTeamId GetGenericTeamId() const { return TeamId; };
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void StartTargeting(UGameplayAbility* Ability) override;
	virtual void Tick(float DeltaTime) override;
	virtual void ConfirmTargetingAndContinue() override;
	virtual void CancelTargeting() override;

private:
	UPROPERTY(Replicated)
	FGenericTeamId TeamId;

	float PullSpeed;
	float BlackHoleDuration;
	FTimerHandle BlackHoleDurationTimerHandle;

	UPROPERTY(ReplicatedUsing = OnRep_BlackHoleRange)
	float BlackHoleRange;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	FName BlackHoleVFXOriginVariableName {"Origin"};

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	class UNiagaraSystem* BlackHoleLinkVFX;

	UFUNCTION()
	void OnRep_BlackHoleRange();
	
	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
	USceneComponent* RootComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
	USphereComponent* DetectionSphereComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
	class UParticleSystemComponent* VFXComponent;

	UFUNCTION()
	void ActorInBlackHoleRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void ActorLeftBlackHoleRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void TryAddTarget(AActor* OtherTarget);
	void RemoveTarget(AActor* OtherTarget);

	TMap<AActor*, class UNiagaraComponent*> ActorsInRangeMap;

	void StopBlackHole();
};

