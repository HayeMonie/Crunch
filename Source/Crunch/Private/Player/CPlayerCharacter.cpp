// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CPlayerCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Crunch/Crunch.h"
#include "GAS/CAbilitySystemStatics.h"
#include "GAS/CHeroAttributeSet.h"
#include "Inventory/InventoryComponent.h"

ACPlayerCharacter::ACPlayerCharacter()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->ProbeChannel = ECC_SpringArm;

	ViewCam = CreateDefaultSubobject<UCameraComponent>(TEXT("View Cam"));
	ViewCam->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

	HeroAttributeSet = CreateDefaultSubobject<UCHeroAttributeSet>("Hero Attribute Set");

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("Inventory Component");
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
		EnhancedInputComp->BindAction(LearnAbilityLeaderAction, ETriggerEvent::Started, this, &ACPlayerCharacter::LearnAbilityLeaderDown);
		EnhancedInputComp->BindAction(LearnAbilityLeaderAction, ETriggerEvent::Completed, this, &ACPlayerCharacter::LearnAbilityLeaderUp);

		for (const TPair<ECAbilityInputID, UInputAction*>& InputActionPair : GameplayAbilityInputActions)
		{
			EnhancedInputComp->BindAction(InputActionPair.Value, ETriggerEvent::Triggered, this, &ACPlayerCharacter::HandleAbilityInput, InputActionPair.Key);

		}

		EnhancedInputComp->BindAction(UseInventoryItemAction, ETriggerEvent::Triggered, this, &ACPlayerCharacter::UseInventoryItem);
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

void ACPlayerCharacter::LearnAbilityLeaderDown(const FInputActionValue& InputActionValue)
{
	bIsLearningAbilityLeaderDown = true;
}

void ACPlayerCharacter::LearnAbilityLeaderUp(const FInputActionValue& InputActionValue)
{
	bIsLearningAbilityLeaderDown = false;
}

void ACPlayerCharacter::UseInventoryItem(const FInputActionValue& InputActionValue)
{
	int Value = FMath::RoundToInt(InputActionValue.Get<float>());
	InventoryComponent->TryActivateItemInSlot(Value - 1);
}

void ACPlayerCharacter::HandleAbilityInput(const FInputActionValue& InputActionValue, ECAbilityInputID InputID)
{
	bool bPressed = InputActionValue.Get<bool>();

	if (bPressed && bIsLearningAbilityLeaderDown)
	{
		UpgradeAbilityWithInputID(InputID);
		return;
	}
	
	if (bPressed)
	{
		GetAbilitySystemComponent()->AbilityLocalInputPressed((int32)InputID);
	}
	else
	{
		GetAbilitySystemComponent()->AbilityLocalInputReleased((int32)InputID);
	}

	if (InputID == ECAbilityInputID::BasicAttack)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, UCAbilitySystemStatics::GetBasicAttackInputPressedTag(), FGameplayEventData());
		Server_SendGameplayEventToSelf(UCAbilitySystemStatics::GetBasicAttackAbilityTag(), FGameplayEventData());
	}
}

void ACPlayerCharacter::SetInputEnabledFromPlayerController(bool bEnabled)
{
	APlayerController* PlayerController = GetController<APlayerController>();
	
	if (!PlayerController)
	{
		return;
	}

	if (bEnabled)
	{
		EnableInput(PlayerController);
	}
	else
	{
		DisableInput(PlayerController);
	}
}

void ACPlayerCharacter::OnStun()
{
	SetInputEnabledFromPlayerController(false);
}

void ACPlayerCharacter::OnRecoverFromStun()
{
	if (IsDead())
	{
		return;
	}
	SetInputEnabledFromPlayerController(true);
}

void ACPlayerCharacter::OnDead()
{
	SetInputEnabledFromPlayerController(false);
}

void ACPlayerCharacter::OnRespawn()
{
	SetInputEnabledFromPlayerController(true);	
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

void ACPlayerCharacter::OnAimStateChanged(bool bIsAiming)
{
	if (IsLocallyControlledByPlayer())
	{
		LerpCameraToLocalOffsetLocation(bIsAiming ? CameraAimLocalOffset : FVector{0.f});		
	}
}

void ACPlayerCharacter::LerpCameraToLocalOffsetLocation(const FVector& Goal)
{
	GetWorldTimerManager().ClearTimer(CameraLerpTimerHandle);
	CameraLerpTimerHandle = GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ACPlayerCharacter::TickCameraLocalOffsetLerp, Goal));
}

void ACPlayerCharacter::TickCameraLocalOffsetLerp(FVector Goal)
{
	FVector CurrentLocalOffset = ViewCam->GetRelativeLocation();
	if (FVector::Dist(CurrentLocalOffset, Goal) < 1.f)
	{
		ViewCam->SetRelativeLocation(Goal);
		return;
	}

	float LerpAlpha = FMath::Clamp(GetWorld()->GetDeltaSeconds() * CameraLerpSpeed, 0.f, 1.f);
	FVector NewLocalOffset = FMath::Lerp(CurrentLocalOffset, Goal, LerpAlpha);
	ViewCam->SetRelativeLocation(NewLocalOffset);
	
	CameraLerpTimerHandle = GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ACPlayerCharacter::TickCameraLocalOffsetLerp, Goal));
}
