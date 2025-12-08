// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GameplayTagContainer.h"
#include "Components/WidgetComponent.h"
#include "CCharacter.generated.h"

UCLASS()
class ACCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACCharacter();
	void ServerSideInit();
	void ClientSideInit();

	virtual void PossessedBy(AController* NewController) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

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

private:
	void BindGASChangeDelegates();
	void DeathTagUpdated(const FGameplayTag Tag, int32 NewCount);
	
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
	/*                               Death And Respawn                                       */
	/*****************************************************************************************/
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
};


