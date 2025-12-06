// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

ACPlayerCharacter::ACPlayerCharacter()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->bUsePawnControlRotation = true;

	ViewCam = CreateDefaultSubobject<UCameraComponent>(TEXT("View Cam"));
	ViewCam->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

}

void ACPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 强制设置，防止蓝图覆盖
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	
	if (CameraBoom)
	{
		CameraBoom->bUsePawnControlRotation = true;
	}
}

void ACPlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	APlayerController* OwningPlayerController = GetController<APlayerController>();
	if (OwningPlayerController)
	{
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem = OwningPlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if (InputSubsystem)
		{
			InputSubsystem->RemoveMappingContext(GameplayInputMappingContext);
			
			InputSubsystem->AddMappingContext(GameplayInputMappingContext, 0);
		}
	}
}

void ACPlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComp = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	if (EnhancedInputComp)
	{
		EnhancedInputComp->BindAction(JumpInputAction, ETriggerEvent::Triggered, this, &ACPlayerCharacter::Jump);
		EnhancedInputComp->BindAction(LookInputAction, ETriggerEvent::Triggered, this, &ACPlayerCharacter::HandleLookInput);
		EnhancedInputComp->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &ACPlayerCharacter::HandleMoveInput);

		for (const TPair<ECAbilityInputID, UInputAction*>& InputActionPair : GameplayAbilityInputActions)
		{
			EnhancedInputComp->BindAction(InputActionPair.Value, ETriggerEvent::Triggered, this, &ACPlayerCharacter::HandleAbilityInput, InputActionPair.Key);

		}
	}
}

void ACPlayerCharacter::HandleLookInput(const FInputActionValue& InputActionValue)
{
	FVector2D InputVal = InputActionValue.Get<FVector2D>();
	AddControllerYawInput(InputVal.X);
	AddControllerPitchInput(-InputVal.Y);

}

void ACPlayerCharacter::HandleMoveInput(const FInputActionValue& InputActionValue)
{
	FVector2D InputVal = InputActionValue.Get<FVector2D>();
	InputVal.Normalize();

	AddMovementInput(GetLookForwardDirection() * InputVal.Y + GetLookRightDirection() * InputVal.X);
}

void ACPlayerCharacter::HandleAbilityInput(const FInputActionValue& InputActionValue, ECAbilityInputID InputID)
{
	bool bPressed = InputActionValue.Get<bool>();
	if (bPressed)
	{
		GetAbilitySystemComponent()->AbilityLocalInputPressed((int32)InputID);
	}
	else
	{
		GetAbilitySystemComponent()->AbilityLocalInputReleased((int32)InputID);
	}
}

FVector ACPlayerCharacter::GetLookRightDirection() const
{
	return ViewCam->GetRightVector();
}

FVector ACPlayerCharacter::GetLookForwardDirection() const
{
	return ViewCam->GetForwardVector();
}

FVector ACPlayerCharacter::GetMoveForwardDirection() const
{
	return FVector::CrossProduct(GetLookRightDirection(), FVector::UpVector);
}
