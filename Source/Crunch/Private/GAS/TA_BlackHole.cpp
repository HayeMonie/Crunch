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

void ATA_BlackHole::OnRep_BlackHoleRange()
{
	DetectionSphereComponent->SetSphereRadius(BlackHoleRange);
}

void ATA_BlackHole::ActorInBlackHoleRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}

void ATA_BlackHole::ActorLeftBlackHoleRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}
