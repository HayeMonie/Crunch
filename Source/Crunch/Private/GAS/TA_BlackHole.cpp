// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/TA_BlackHole.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

ATA_BlackHole::ATA_BlackHole()
{
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root Comp"));
	SetRootComponent(RootComp);

	DetectionSphereComponent = CreateDefaultSubobject<USphereComponent>("Detection Sphere Component");
	DetectionSphereComponent->SetupAttachment(GetRootComponent());
	DetectionSphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	DetectionSphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	DetectionSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ATA_BlackHole::ActorInBlackHoleRange);
	DetectionSphereComponent->OnComponentEndOverlap.AddDynamic(this, &ATA_BlackHole::ActorLeftBlackHoleRange);

	bReplicates = true;
	ShouldProduceTargetDataOnServer = true;
	PrimaryActorTick.bCanEverTick = true;

	VFXComponent = CreateDefaultSubobject<UParticleSystemComponent>("VFX Component");
	VFXComponent->SetupAttachment(GetRootComponent());
}

void ATA_BlackHole::ConfigureBlackHole(float InBlackHoleRange, float InPullSpeed, float InBlackHoleDuration,
	const FGenericTeamId& InTeamId)
{
	PullSpeed = InPullSpeed;
	DetectionSphereComponent->SetSphereRadius(InBlackHoleRange);
	SetGenericTeamId(InTeamId);
	BlackHoleDuration = InBlackHoleDuration;
	BlackHoleRange = InBlackHoleRange;
}

void ATA_BlackHole::SetGenericTeamId(const FGenericTeamId& InTeamId)
{
	TeamId = InTeamId;
	
}

void ATA_BlackHole::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATA_BlackHole, TeamId);
	DOREPLIFETIME_CONDITION_NOTIFY(ATA_BlackHole, BlackHoleRange, COND_None, REPNOTIFY_Always);
}

void ATA_BlackHole::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(BlackHoleDurationTimerHandle, this, &ATA_BlackHole::StopBlackHole, BlackHoleDuration, false);
	}
}

void ATA_BlackHole::Tick(float DeltaTime)
{
	if (HasAuthority())
	{
		for (auto& TargetPair : ActorsInRangeMap)
		{
			AActor* Target = TargetPair.Key;
			UNiagaraComponent* NiagaraComponent = TargetPair.Value;

			FVector PullDirection = (GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();
			Target->SetActorLocation(Target->GetActorLocation() + PullDirection * PullSpeed * DeltaTime);

			if (NiagaraComponent)
			{
				NiagaraComponent->SetVariablePosition(BlackHoleVFXOriginVariableName, VFXComponent->GetComponentLocation());
			}
		}
	}
}

void ATA_BlackHole::ConfirmTargetingAndContinue()
{
	StopBlackHole();
}

void ATA_BlackHole::CancelTargeting()
{
	StopBlackHole();
	Super::CancelTargeting();
}

void ATA_BlackHole::OnRep_BlackHoleRange()
{
	DetectionSphereComponent->SetSphereRadius(BlackHoleRange);
}

void ATA_BlackHole::ActorInBlackHoleRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TryAddTarget(OtherActor);
}

void ATA_BlackHole::ActorLeftBlackHoleRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	RemoveTarget(OtherActor);
}

void ATA_BlackHole::TryAddTarget(AActor* OtherTarget)
{
	if (!OtherTarget || ActorsInRangeMap.Contains(OtherTarget))
	{
		return;
	}

	if (GetTeamAttitudeTowards(*OtherTarget) != ETeamAttitude::Hostile)
	{
		return;
	}

	UNiagaraComponent* NiagaraComponent = nullptr;
	if (BlackHoleLinkVFX)
	{
		NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(BlackHoleLinkVFX, OtherTarget->GetRootComponent(), NAME_None,
			FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false);
		if (NiagaraComponent)
		{
			NiagaraComponent->SetVariablePosition(BlackHoleVFXOriginVariableName, VFXComponent->GetComponentLocation());
		}
	}

	ActorsInRangeMap.Add(OtherTarget, NiagaraComponent);
}

void ATA_BlackHole::RemoveTarget(AActor* OtherTarget)
{
	if (!OtherTarget)
	{
		return;
	}

	if (ActorsInRangeMap.Contains(OtherTarget))
	{
		UNiagaraComponent* VFXComp;
		ActorsInRangeMap.RemoveAndCopyValue(OtherTarget, VFXComp);
		if (IsValid(VFXComp))
		{
			VFXComp->DestroyComponent();
		}
	}
}

void ATA_BlackHole::StopBlackHole()
{
	TArray<TWeakObjectPtr<AActor>> FinalTargets;
	for (TPair<AActor*, UNiagaraComponent*>& TargetPair : ActorsInRangeMap)
	{
		FinalTargets.Add(TargetPair.Key);
		UNiagaraComponent* NiagaraComponent = TargetPair.Value;
		if (IsValid(NiagaraComponent))
		{
			NiagaraComponent->DestroyComponent();
		}

	}
	
	FGameplayAbilityTargetDataHandle TargetDataHandle;

	FGameplayAbilityTargetData_ActorArray* TargetActorArray = new FGameplayAbilityTargetData_ActorArray;
	TargetActorArray->SetActors(FinalTargets);
	TargetDataHandle.Add(TargetActorArray);
	
	FGameplayAbilityTargetData_SingleTargetHit* BlowUpLocation = new FGameplayAbilityTargetData_SingleTargetHit;
	BlowUpLocation->HitResult.ImpactPoint = GetActorLocation();
	TargetDataHandle.Add(BlowUpLocation);

	TargetDataReadyDelegate.Broadcast(TargetDataHandle);
}
