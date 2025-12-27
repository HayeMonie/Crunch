// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CCharacter.h"
#include "Crunch/Crunch.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Widgets/ValueGauge.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/CAbilitySystemComponent.h"
#include "GAS/CAttributeSet.h"
#include "GAS/CHeroAttributeSet.h"
#include "GAS/CAbilitySystemStatics.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Sight.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/OverHeadStatsGauge.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"


// Sets default values
ACCharacter::ACCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_SpringArm, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Target, ECR_Ignore);

	CAbilitySystemComponent = CreateDefaultSubobject<UCAbilitySystemComponent>(TEXT("CAbilitySystemComponent"));
	CAttributeSet = CreateDefaultSubobject<UCAttributeSet>(TEXT("CAttribute Set"));
	CHeroAttributeSet = CreateDefaultSubobject<UCHeroAttributeSet>(TEXT("CHeroAttributeSet"));

	// OverHeadWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Over Head Widget Component"));
	// OverHeadWidgetComponent->SetupAttachment(GetRootComponent());

	PerceptionStimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("Perception Stimuli Source Component"));

}

void ACCharacter::ServerSideInit()
{
	if (!CAbilitySystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("CAbilitySystemComponent is NULL in ServerSideInit for %s"), *GetName());
		return;
	}
	
	CAbilitySystemComponent->InitAbilityActorInfo(this, this);
	// 绑定死亡标签变化委托，监听角色死亡和重生
	BindGASChangeDelegates();

	CAbilitySystemComponent->ServerSideInit();
}

void ACCharacter::ClientSideInit()
{
	CAbilitySystemComponent->InitAbilityActorInfo(this, this);
	// 客户端也需要绑定委托以响应服务器复制的死亡标签变化
	BindGASChangeDelegates();
}

void ACCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACCharacter, TeamID);
}

const TMap<ECAbilityInputID, TSubclassOf<UGameplayAbility>>& ACCharacter::GetAbilities() const
{
	return CAbilitySystemComponent->GetAbilities();
}

void ACCharacter::PossessedBy(class AController* NewController)
{
	Super::PossessedBy(NewController);
	if (NewController && !NewController->IsPlayerController() && CAbilitySystemComponent)
	{
		ServerSideInit();
	}
}

// Called when the game starts or when spawned
void ACCharacter::BeginPlay()
{
	Super::BeginPlay();
	ConfigureOverHeadStatusWidget();

	MeshRelativeTransform = GetMesh()->GetRelativeTransform();

	PerceptionStimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());

	// 客户端初始化GAS，确保能接收并响应服务器复制的标签变化
	if (!HasAuthority() && CAbilitySystemComponent)
	{
		ClientSideInit();
	}
}

// Called every frame
void ACCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

UAbilitySystemComponent* ACCharacter::GetAbilitySystemComponent() const
{
	return CAbilitySystemComponent;
}

void ACCharacter::Server_SendGameplayEventToSelf_Implementation(const FGameplayTag& EventTag,
	const FGameplayEventData& EventData)
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventTag, EventData);
}

bool ACCharacter::Server_SendGameplayEventToSelf_Validate(const FGameplayTag& EventTag,
	const FGameplayEventData& EventData)
{
	return true;
}

bool ACCharacter::IsLocallyControlledByPlayer() const
{
	return GetController() != nullptr && GetController()->IsLocalPlayerController();
}

void ACCharacter::BindGASChangeDelegates()
{
	if (CAbilitySystemComponent)
	{
		CAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).AddUObject(this, &ACCharacter::DeathTagUpdated);
		CAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetStunStatTag()).AddUObject(this, &ACCharacter::StunTagUpdated);
		CAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetAimStatTag()).AddUObject(this, &ACCharacter::AimTagUpdated);

		CAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetMoveSpeedAttribute()).AddUObject(this, &ACCharacter::MoveSpeedUpdated);
		CAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &ACCharacter::MaxHealthUpdated);
		CAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetMaxManaAttribute()).AddUObject(this, &ACCharacter::MaxManaUpdated);
	}
}

void ACCharacter::DeathTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount != 0)
	{
		StartDeathSequence();
	}
	else
	{
		Respawn();
	}
}

void ACCharacter::StunTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if (IsDead())
	{
		return;
	}

	if (NewCount != 0)
	{
		// if (CAbilitySystemComponent)
		// {
		// 	CAbilitySystemComponent->CancelAllAbilities();
		// }
		OnStun();
		PlayAnimMontage(StunMontage);
	}
	else
	{
		OnRecoverFromStun();
		StopAnimMontage(StunMontage);
	}
}

void ACCharacter::AimTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	SetIsAiming(NewCount != 0);
}

void ACCharacter::SetIsAiming(bool bIsAiming)
{
	bUseControllerRotationYaw = bIsAiming;
	GetCharacterMovement()->bOrientRotationToMovement = !bIsAiming;
	OnAimStateChanged(bIsAiming);
}

void ACCharacter::OnAimStateChanged(bool bIsAiming)
{
	// Override in child class
}

void ACCharacter::MoveSpeedUpdated(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
}

void ACCharacter::MaxHealthUpdated(const FOnAttributeChangeData& Data)
{
	if (!HasAuthority())
	{
		return;
	}
	
	UCAttributeSet* AttributeSet = const_cast<UCAttributeSet*>(CAbilitySystemComponent->GetSet<UCAttributeSet>());
	if (AttributeSet)
	{
		AttributeSet->RescaleHealth();
	}
}

void ACCharacter::MaxManaUpdated(const FOnAttributeChangeData& Data)
{
	if (!HasAuthority())
	{
		return;
	}
	
	UCAttributeSet* AttributeSet = const_cast<UCAttributeSet*>(CAbilitySystemComponent->GetSet<UCAttributeSet>());
	if (AttributeSet)
	{
		AttributeSet->RescaleMana();
	}
}

void ACCharacter::ConfigureOverHeadStatusWidget()
{
	OverHeadWidgetComponent = FindComponentByClass<UWidgetComponent>();	

	if (!OverHeadWidgetComponent) return;

	if (IsLocallyControlledByPlayer())
	{
		OverHeadWidgetComponent->SetHiddenInGame(true);
		return;
	}

	UOverHeadStatsGauge* OverHeadStatsGauge = Cast<UOverHeadStatsGauge>(OverHeadWidgetComponent->GetUserWidgetObject());
	if (OverHeadStatsGauge)
	{
		OverHeadStatsGauge->ConfigureWithASC(GetAbilitySystemComponent());
		OverHeadWidgetComponent->SetHiddenInGame(false);

		GetWorldTimerManager().ClearTimer(HeadStatGaugeVisibilityUpdateTimerHandle);
		GetWorldTimerManager().SetTimer(HeadStatGaugeVisibilityUpdateTimerHandle, this,
			&ACCharacter::UpdateHeadGaugeVisibility, HeadStatGaugeVisibilityUpdateGap, true);
	}
}

void ACCharacter::UpdateHeadGaugeVisibility()
{
	APawn* LocalPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (LocalPlayerPawn)
	{
		float DistSquared = FVector::DistSquared(GetActorLocation(), LocalPlayerPawn->GetActorLocation());
		OverHeadWidgetComponent->SetHiddenInGame(DistSquared > HeadStatGaugeVisibilityRangeSquared);
	}
}

void ACCharacter::SetStatusGaugeEnabled(bool bIsEnabled)
{
	GetWorldTimerManager().ClearTimer(HeadStatGaugeVisibilityUpdateTimerHandle);

	if (bIsEnabled)
	{
		ConfigureOverHeadStatusWidget();
	}
	else
	{
		OverHeadWidgetComponent->SetHiddenInGame(true);
	}
}

void ACCharacter::OnStun()
{
}

void ACCharacter::OnRecoverFromStun()
{
}

bool ACCharacter::IsDead() const
{
	return GetAbilitySystemComponent()->HasMatchingGameplayTag(UCAbilitySystemStatics::GetDeadStatTag());  
}

void ACCharacter::RespawnImmediately()
{
	if (HasAuthority())  
	{
		GetAbilitySystemComponent()->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(UCAbilitySystemStatics::GetDeadStatTag()));  
	}
}

void ACCharacter::DeathMontageFinished()
{
	SetRagdollEnabled(true);
}

void ACCharacter::SetRagdollEnabled(bool bIsEnabled)
{
	if (bIsEnabled)
	{
		GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	}
	else
	{
		GetMesh()->SetSimulatePhysics(false);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		GetMesh()->SetRelativeTransform(MeshRelativeTransform);
	}
}

void ACCharacter::PlayDeathAnimation()
{
	if (DeathMontage)
	{
		float MontageDuration = PlayAnimMontage(DeathMontage);
		GetWorldTimerManager().SetTimer(DeathMontageTimerHandle, this, &ACCharacter::DeathMontageFinished, MontageDuration + DeathMontageFinishTimeShift);
	}
	
}

void ACCharacter::StartDeathSequence()
{
	OnDead();

	if (CAbilitySystemComponent)
	{
		CAbilitySystemComponent->CancelAllAbilities();	
	}
	
	PlayDeathAnimation();
	SetStatusGaugeEnabled(false);
	
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetAIPerceptionStimuliSourceEnabled(false);
}

void ACCharacter::Respawn()
{
	OnRespawn();
	SetAIPerceptionStimuliSourceEnabled(true);
	SetRagdollEnabled(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	GetMesh()->GetAnimInstance()->StopAllMontages(0.f);
	SetStatusGaugeEnabled(true);

	if (HasAuthority() && GetController())
	{
		TWeakObjectPtr<AActor> StartSpot = GetController()->StartSpot;
		if (StartSpot.IsValid())
		{
			SetActorTransform(StartSpot->GetActorTransform());
		}
	}
	
	if (CAbilitySystemComponent)
	{
		CAbilitySystemComponent->ApplyFullStatEffect();
	}
}

void ACCharacter::OnDead()
{
}

void ACCharacter::OnRespawn()
{
}

void ACCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	IGenericTeamAgentInterface::SetGenericTeamId(NewTeamID);

	TeamID = NewTeamID;
}

FGenericTeamId ACCharacter::GetGenericTeamId() const
{
	return TeamID;
}

void ACCharacter::OnRep_TeamID()
{
	// override in child class
}

void ACCharacter::SetAIPerceptionStimuliSourceEnabled(bool bIsEnabled)
{
	if (!PerceptionStimuliSourceComponent)
	{
		return;
	}

	if (bIsEnabled)
	{
		PerceptionStimuliSourceComponent->RegisterWithPerceptionSystem();
	}
	else
	{
		PerceptionStimuliSourceComponent->UnregisterFromPerceptionSystem();
	}
}
