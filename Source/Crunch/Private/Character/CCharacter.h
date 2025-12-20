// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Components/WidgetComponent.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "CCharacter.generated.h"

UCLASS()
class ACCharacter : public ACharacter, public IAbilitySystemInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACCharacter();
	void ServerSideInit();
	void ClientSideInit();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	const TMap<ECAbilityInputID, TSubclassOf<UGameplayAbility>>& GetAbilities() const;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/*****************************************************************************************/
	/*                                 Gameplay Ability                                      */
	/*****************************************************************************************/
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendGameplayEventToSelf(const FGameplayTag& EventTag, const FGameplayEventData& EventData);

private:
	void BindGASChangeDelegates();
	void DeathTagUpdated(const FGameplayTag Tag, int32 NewCount);
	void StunTagUpdated(const FGameplayTag Tag, int32 NewCount);
	
	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay Ability")
	class UCAbilitySystemComponent* CAbilitySystemComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay Ability")
	class UCAttributeSet* CAttributeSet;

	/*****************************************************************************************/
	/*                                         UI                                            */
	/*****************************************************************************************/
public:
	bool IsLocallyControlledByPlayer() const;
	
private:
	void ConfigureOverHeadStatusWidget();

	UPROPERTY()
	UWidgetComponent* OverHeadWidgetComponent;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float HeadStatGaugeVisibilityUpdateGap {1.f};

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float HeadStatGaugeVisibilityRangeSquared {1000000.f};

	FTimerHandle HeadStatGaugeVisibilityUpdateTimerHandle;

	void UpdateHeadGaugeVisibility();
	void SetStatusGaugeEnabled(bool bIsEnabled);

	/*****************************************************************************************/
	/*                                       Stun                                            */
	/*****************************************************************************************/
private:
	UPROPERTY(EditDefaultsOnly, Category = "Stun")
	UAnimMontage* StunMontage;

	virtual void OnStun();
	virtual void OnRecoverFromStun();
	

	/*****************************************************************************************/
	/*                               Death And Respawn                                       */
	/*****************************************************************************************/
public:
	// 判断角色是否处于死亡状态（基于DeathTag）
	bool IsDead() const;

	// 立即重生（封装重生逻辑，仅服务端执行）
	void RespawnImmediately();
	
	
private:
	FTransform MeshRelativeTransform;
	
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeathMontageFinishTimeShift {-0.78f};

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	UAnimMontage* DeathMontage;

	FTimerHandle DeathMontageTimerHandle;

	void DeathMontageFinished();
	void SetRagdollEnabled(bool bIsEnabled);

	void PlayDeathAnimation();
	
	void StartDeathSequence();
	void Respawn();

	virtual void OnDead();
	virtual void OnRespawn();

	/*****************************************************************************************/
	/*                                        Team                                           */
	/*****************************************************************************************/

public:
	/** Assigns Team Agent to given TeamID */
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	
	/** Retrieve team identifier in form of FGenericTeamId */
	virtual FGenericTeamId GetGenericTeamId() const override;

private:
	UPROPERTY(ReplicatedUsing = OnRep_TeamID)
	FGenericTeamId TeamID;

	UFUNCTION()
	virtual void OnRep_TeamID();

	/*****************************************************************************************/
	/*                                         AI                                            */
	/*****************************************************************************************/
private:
	void SetAIPerceptionStimuliSourceEnabled(bool bIsEnabled);
	
	UPROPERTY()
	class UAIPerceptionStimuliSourceComponent* PerceptionStimuliSourceComponent;

	
};


