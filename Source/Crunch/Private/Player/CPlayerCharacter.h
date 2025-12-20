// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputSubsystemInterface.h"
#include "InputActionValue.h"
#include "Character/CCharacter.h"
#include "GAS/CAbilitySystemComponent.h"
#include "CPlayerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class ACPlayerCharacter : public ACCharacter
{
	GENERATED_BODY()

public:
	ACPlayerCharacter();
	virtual void BeginPlay() override;
	virtual void PawnClientRestart() override;   // 多人游戏中 初始化客户端中的输入系统
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
private:
	UPROPERTY(VisibleDefaultsOnly, Category = "View")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleDefaultsOnly, Category = "View")
	class UCameraComponent* ViewCam;
	
	FVector GetLookRightDirection() const;
	FVector GetLookForwardDirection() const;
	FVector GetMoveForwardDirection() const;

	
	/**********************************************************************/
	/*                               Input                                */
	/**********************************************************************/
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* JumpInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* LookInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* MoveInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TMap<ECAbilityInputID, class UInputAction*> GameplayAbilityInputActions;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* GameplayInputMappingContext;

	void HandleLookInput(const FInputActionValue& InputActionValue);
	void HandleMoveInput(const FInputActionValue& InputActionValue);
	void HandleAbilityInput(const FInputActionValue& InputActionValue, ECAbilityInputID InputID);
	void SetInputEnabledFromPlayerController(bool bEnabled);
	
	/**********************************************************************/
	/*                                Stun                                */
	/**********************************************************************/
	virtual void OnStun() override;
	virtual void OnRecoverFromStun() override;

	

	/**********************************************************************/
	/*                          Death And Respawn                         */
	/**********************************************************************/
	virtual void OnDead() override;
	virtual void OnRespawn() override;
	
};
