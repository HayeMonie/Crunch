// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/TargetActor_Line.h"
#include "Crunch/Crunch.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "Net/UnrealNetwork.h"
#include "Abilities/GameplayAbility.h"
#include "Kismet/KismetMathLibrary.h"

ATargetActor_Line::ATargetActor_Line()
{
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root Comp"));
	SetRootComponent(RootComp);

	LazerCollisionCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Lazer Collision Capsule"));
	LazerCollisionCapsule->SetupAttachment(GetRootComponent());

	// 启用碰撞体并设置为查询模式以支持重叠检测
	LazerCollisionCapsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LazerCollisionCapsule->SetCollisionObjectType(ECC_WorldDynamic);
	LazerCollisionCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	LazerCollisionCapsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	LazerCollisionCapsule->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	LazerCollisionCapsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	LazerCollisionCapsule->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	LazerCollisionCapsule->SetCollisionResponseToChannel(ECC_SpringArm, ECR_Ignore);

	LazerVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Lazer VFX"));
	LazerVFX->SetupAttachment(GetRootComponent());

	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	ShouldProduceTargetDataOnServer = true;

	AvatarActor = nullptr;

	// 设置默认值
	TargetRange = 400.0f;
	DetectionCylinderRadius = 30.0f;
	TargetingInterval = 0.3f;
}

void ATargetActor_Line::ConfigureTargetSetting(float NewTargetRange, float NewDetectionCylinderRadius,
	float NewTargetingInterval, FGenericTeamId OwnerTeamID, bool bShouldDrawDebug)
{
	// 只有当传入的值大于0时才覆盖，否则使用蓝图中的默认值
	if (NewTargetRange > 0.0f)
	{
		TargetRange = NewTargetRange;
	}
	if (NewDetectionCylinderRadius > 0.0f)
	{
		DetectionCylinderRadius = NewDetectionCylinderRadius;
	}
	if (NewTargetingInterval > 0.0f)
	{
		TargetingInterval = NewTargetingInterval;
	}
	SetGenericTeamId(OwnerTeamID);
	bDrawDebug = bShouldDrawDebug;
}

void ATargetActor_Line::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID = NewTeamID;
}

void ATargetActor_Line::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATargetActor_Line, TeamID);
	DOREPLIFETIME(ATargetActor_Line, TargetRange);
	DOREPLIFETIME(ATargetActor_Line, DetectionCylinderRadius);
	DOREPLIFETIME(ATargetActor_Line, AvatarActor);
}

void ATargetActor_Line::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);
	if (!OwningAbility)
	{
		return;
	}

	AvatarActor = OwningAbility->GetAvatarActorFromActorInfo();
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(PeriodicTargetingTimerHandle, this, &ATargetActor_Line::DoTargetCheckAndReport, TargetingInterval, true);
	}
}

void ATargetActor_Line::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 每帧修正lazer位置，确保它始终在Socket中心
	// 通过将RootComponent的相对位置设置为(0,0,0)，强制与Socket位置同步
	if (GetRootComponent() && GetRootComponent()->GetAttachParent())
	{
		// 重置相对位置到原点，使其紧贴在Socket位置
		GetRootComponent()->SetRelativeLocation(FVector::ZeroVector, false, nullptr, ETeleportType::TeleportPhysics);
	}

	// 每帧进行碰撞检测和截断
	UpdateTargetTrace();
}

void ATargetActor_Line::BeginDestroy()
{
	if (GetWorld() && PeriodicTargetingTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(PeriodicTargetingTimerHandle);
	}
	Super::BeginDestroy();
	
}

void ATargetActor_Line::UpdateTargetTrace()
{
	FVector ViewLocation = GetActorLocation();
	FRotator ViewRotation = GetActorRotation();
	if (AvatarActor)
	{
		AvatarActor->GetActorEyesViewPoint(ViewLocation, ViewRotation);
	}

	FVector LookEndPoint = ViewLocation + ViewRotation.Vector() * 100000.f;
	FRotator LookRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), LookEndPoint);
	SetActorRotation(LookRotation);

	// 碰撞检测的起始位置需要考虑前向偏移和Z轴偏移
	FVector ForwardDirection = LookRotation.Vector();
	FVector ZOffset = FVector::UpVector * CollisionZOffset;
	FVector CollisionStartLocation = GetActorLocation() + ForwardDirection * CollisionStartOffset + ZOffset;
	FVector SweepEndLocation = CollisionStartLocation + ForwardDirection * TargetRange;

	// 收集所有碰撞结果
	TArray<FHitResult> AllHitResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(AvatarActor);
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;

	// 使用 SweepMultiByObjectType 检测多种对象类型
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	// 使用胶囊体碰撞（圆柱体形状），高度为TargetRange
	float CapsuleHalfHeight = TargetRange / 2.0f;
	TArray<FHitResult> ObjectHits;
	GetWorld()->SweepMultiByObjectType(
		ObjectHits,
		CollisionStartLocation,
		SweepEndLocation,
		FQuat::Identity,
		ObjectTypes,
		FCollisionShape::MakeCapsule(DetectionCylinderRadius, CapsuleHalfHeight),
		QueryParams
	);

	AllHitResults.Append(ObjectHits);

	// 调试信息
	if (bDrawDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("=== LAZER COLLISION DEBUG ==="));
		UE_LOG(LogTemp, Warning, TEXT("Actor Location: %s, Collision Start Location: %s, Range: %f, Radius: %f"),
			*GetActorLocation().ToString(), *CollisionStartLocation.ToString(), TargetRange, DetectionCylinderRadius);
		UE_LOG(LogTemp, Warning, TEXT("Collision Start Offset (Forward): %f"), CollisionStartOffset);
		UE_LOG(LogTemp, Warning, TEXT("Collision Z Offset: %f"), CollisionZOffset);
		UE_LOG(LogTemp, Warning, TEXT("Total Object Hits: %d"), AllHitResults.Num());

		// 绘制从Actor位置到碰撞体起始位置的连线
		DrawDebugLine(GetWorld(), GetActorLocation(), CollisionStartLocation, FColor::Yellow, false, -1.0f, 0, 1.0f);

		// 绘制完整的射线
		DrawDebugLine(GetWorld(), GetActorLocation(), SweepEndLocation, FColor::Red, false, -1.0f, 0, 2.0f);

		// 绘制胶囊体碰撞体积（圆柱体）
		DrawDebugCapsule(GetWorld(),
			GetActorLocation() + LookRotation.Vector() * CapsuleHalfHeight,
			CapsuleHalfHeight,
			DetectionCylinderRadius,
			LookRotation.Quaternion(),
			FColor::Cyan,
			false,
			-1.0f
		);

		// 输出每个碰撞的详细信息
		for (int32 i = 0; i < AllHitResults.Num(); i++)
		{
			const FHitResult& Hit = AllHitResults[i];
			ECollisionChannel Channel = Hit.GetComponent()->GetCollisionObjectType();
			FString ChannelName = "Unknown";
			if (Channel == ECC_WorldStatic) ChannelName = "WorldStatic";
			else if (Channel == ECC_WorldDynamic) ChannelName = "WorldDynamic";
			else if (Channel == ECC_Pawn) ChannelName = "Pawn";

			ETeamAttitude::Type Attitude = GetTeamAttitudeTowards(*Hit.GetActor());
			FString AttitudeStr = "Unknown";
			if (Attitude == ETeamAttitude::Friendly) AttitudeStr = "Friendly";
			else if (Attitude == ETeamAttitude::Hostile) AttitudeStr = "Hostile";
			else if (Attitude == ETeamAttitude::Neutral) AttitudeStr = "Neutral";

			UE_LOG(LogTemp, Warning, TEXT("  Hit %d: %s, Channel=%s, Dist=%.2f, Team=%s"),
				i, *Hit.GetActor()->GetName(), *ChannelName, Hit.Distance, *AttitudeStr);
		}
	}

	// 按距离排序碰撞结果，找到最近的碰撞点
	AllHitResults.Sort([](const FHitResult& A, const FHitResult& B)
	{
		return A.Distance < B.Distance;
	});

	FVector LineEndLocation = SweepEndLocation;
	float LineLength = TargetRange;
	bool bShouldBlockLazer = false;

	// 碰撞检测后的实际起点也需要考虑偏移
	FVector CollisionLineStart = CollisionStartLocation;

	// 注释掉截断逻辑，只保留碰撞检测
	/*
	for (const FHitResult& HitResult : AllHitResults)
	{
		if (HitResult.GetActor())
		{
			ECollisionChannel ObjectType = HitResult.GetComponent()->GetCollisionObjectType();

			// 如果是 WorldStatic 中的障碍物，直接阻挡
			if (ObjectType == ECC_WorldStatic)
			{
				bShouldBlockLazer = true;
				LineEndLocation = HitResult.ImpactPoint;
				LineLength = FVector::Distance(GetActorLocation(), LineEndLocation);
				if (bDrawDebug)
				{
					UE_LOG(LogTemp, Warning, TEXT(">>> BLOCKED by WorldStatic: %s"), *HitResult.GetActor()->GetName());
					DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 15.0f, FColor::Red, false, -1.0f);
				}
				break;
			}

			// 检查是否是敌对目标
			ETeamAttitude::Type Attitude = GetTeamAttitudeTowards(*HitResult.GetActor());
			if (Attitude != ETeamAttitude::Friendly)
			{
				bShouldBlockLazer = true;
				LineEndLocation = HitResult.ImpactPoint;
				LineLength = FVector::Distance(GetActorLocation(), LineEndLocation);
				if (bDrawDebug)
				{
					UE_LOG(LogTemp, Warning, TEXT(">>> BLOCKED by Enemy: %s (Attitude=%d)"), *HitResult.GetActor()->GetName(), (int32)Attitude);
					DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 15.0f, FColor::Orange, false, -1.0f);
				}
				break;
			}
		}
	}
	*/

	// 注释掉根据截断更新碰撞体和VFX的逻辑
	/*
	// 更新碰撞体组件的位置和大小以匹配实际的激光长度
	if (LazerCollisionCapsule)
	{
		float ActualCapsuleHalfHeight = LineLength / 2.0f;
		FVector CapsuleCenter = GetActorLocation() + LookRotation.Vector() * ActualCapsuleHalfHeight;

		LazerCollisionCapsule->SetCapsuleRadius(DetectionCylinderRadius);
		LazerCollisionCapsule->SetCapsuleHalfHeight(ActualCapsuleHalfHeight);
		LazerCollisionCapsule->SetWorldLocation(CapsuleCenter);
		LazerCollisionCapsule->SetWorldRotation(LookRotation);
	}

	if (LazerVFX)
	{
		LazerVFX->SetVariableFloat(LazerFXLengthParamName, LineLength / 100.f);
	}
	*/

	// 保留碰撞体和VFX的完整长度更新
	if (LazerCollisionCapsule)
	{
		float ActualCapsuleHalfHeight = LineLength / 2.0f;

		// 碰撞体的起始位置需要考虑前向偏移和Z轴偏移（使用前面声明的变量）
		FVector CapsuleStartLocation = GetActorLocation() + ForwardDirection * CollisionStartOffset + ZOffset;
		FVector CapsuleCenter = CapsuleStartLocation + ForwardDirection * ActualCapsuleHalfHeight;
		FVector CapsuleStart = CapsuleStartLocation;
		FVector CapsuleEnd = CapsuleStartLocation + ForwardDirection * LineLength;

		LazerCollisionCapsule->SetCapsuleRadius(DetectionCylinderRadius);
		LazerCollisionCapsule->SetCapsuleHalfHeight(ActualCapsuleHalfHeight);
		LazerCollisionCapsule->SetWorldLocation(CapsuleCenter);

		// 修复：CapsuleComponent的高度方向默认是Z轴，我们需要旋转使其沿着Lazer方向
		// 使用 MakeRotFromZY：Z轴（Capsule高度方向）对齐到Lazer方向，Y轴保持向上
		FRotator CapsuleRotation = UKismetMathLibrary::MakeRotFromZY(LookRotation.Vector(), FVector::UpVector);
		LazerCollisionCapsule->SetWorldRotation(CapsuleRotation);

		// Debug: 输出碰撞体的详细信息
		if (bDrawDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("=== CAPSULE COLLISION DEBUG ==="));
			UE_LOG(LogTemp, Warning, TEXT("Look Rotation: %s"), *LookRotation.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Look Direction: %s"), *LookRotation.Vector().ToString());
			UE_LOG(LogTemp, Warning, TEXT("Collision Start Offset (Forward): %f"), CollisionStartOffset);
			UE_LOG(LogTemp, Warning, TEXT("Collision Z Offset: %f"), CollisionZOffset);
			UE_LOG(LogTemp, Warning, TEXT("Capsule Rotation: %s"), *CapsuleRotation.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Capsule Start Location: %s"), *CapsuleStartLocation.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Capsule Start: %s"), *CapsuleStart.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Capsule End: %s"), *CapsuleEnd.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Capsule Center: %s"), *CapsuleCenter.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Capsule Length: %f"), LineLength);
			UE_LOG(LogTemp, Warning, TEXT("Capsule Radius: %f"), DetectionCylinderRadius);

			// 绘制碰撞体的起点和终点
			DrawDebugPoint(GetWorld(), CapsuleStartLocation, 20.0f, FColor::Yellow, false, -1.0f);
			DrawDebugPoint(GetWorld(), CapsuleStart, 20.0f, FColor::Green, false, -1.0f);
			DrawDebugPoint(GetWorld(), CapsuleEnd, 20.0f, FColor::Red, false, -1.0f);

			// 绘制胶囊体的边界框
			FBoxSphereBounds Bounds = LazerCollisionCapsule->CalcBounds(LazerCollisionCapsule->GetComponentTransform());
			DrawDebugBox(GetWorld(), Bounds.Origin, Bounds.BoxExtent, FQuat::Identity, FColor::Yellow, false, -1.0f, 0, 2.0f);

			// 检查碰撞体的实际世界位置
			FVector ActualCapsuleLocation = LazerCollisionCapsule->GetComponentLocation();
			FRotator ActualCapsuleRotation = LazerCollisionCapsule->GetComponentRotation();
			UE_LOG(LogTemp, Warning, TEXT("Actual Capsule World Location: %s"), *ActualCapsuleLocation.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Actual Capsule World Rotation: %s"), *ActualCapsuleRotation.ToString());

			// 计算碰撞体的各个轴向
			FVector CapsuleForward = ActualCapsuleRotation.Vector();
			FVector CapsuleRight = FRotationMatrix(ActualCapsuleRotation).GetScaledAxis(EAxis::Y);
			FVector CapsuleUp = FRotationMatrix(ActualCapsuleRotation).GetScaledAxis(EAxis::Z);

			UE_LOG(LogTemp, Warning, TEXT("Capsule Forward (X): %s"), *CapsuleForward.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Capsule Right (Y): %s"), *CapsuleRight.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Capsule Up (Z, Height): %s"), *CapsuleUp.ToString());

			// 计算并输出碰撞体的实际起点和终点（基于碰撞体的世界位置和旋转）
			FVector ActualCapsuleStart = ActualCapsuleLocation - CapsuleUp * ActualCapsuleHalfHeight;
			FVector ActualCapsuleEnd = ActualCapsuleLocation + CapsuleUp * ActualCapsuleHalfHeight;
			UE_LOG(LogTemp, Warning, TEXT("Actual Capsule Start: %s"), *ActualCapsuleStart.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Actual Capsule End: %s"), *ActualCapsuleEnd.ToString());

			// 绘制实际的碰撞体范围
			DrawDebugCapsule(GetWorld(),
				ActualCapsuleLocation,
				ActualCapsuleHalfHeight,
				DetectionCylinderRadius,
				ActualCapsuleRotation.Quaternion(),
				FColor::Magenta,
				false,
				-1.0f,
				0,
				2.0f
			);
		}
	}

	if (LazerVFX)
	{
		LazerVFX->SetVariableFloat(LazerFXLengthParamName, LineLength / 100.f);
	}

	// 绘制最终的激光线
	if (bDrawDebug)
	{
		FColor LineColor = bShouldBlockLazer ? FColor::Green : FColor::Purple;
		DrawDebugLine(GetWorld(), CollisionLineStart, LineEndLocation, LineColor, false, -1.0f, 0, 8.0f);

		// 绘制实际碰撞体积
		float ActualCapsuleHalfHeight = LineLength / 2.0f;
		DrawDebugCapsule(GetWorld(),
			CollisionLineStart + LookRotation.Vector() * ActualCapsuleHalfHeight,
			ActualCapsuleHalfHeight,
			DetectionCylinderRadius,
			LookRotation.Quaternion(),
			LineColor,
			false,
			-1.0f
		);

		UE_LOG(LogTemp, Warning, TEXT("=== RESULT: Length=%.2f, Blocked=%s ==="), LineLength, bShouldBlockLazer ? TEXT("YES") : TEXT("NO"));
	}
}

bool ATargetActor_Line::ShouldReportActorAsTarget(const AActor* ActorToCheck) const
{
	if (!ActorToCheck)
	{
		return false;
	}

	if (ActorToCheck == AvatarActor)
	{
		return false;
	}

	if (ActorToCheck == this)
	{
		return false;
	}

	if (GetTeamAttitudeTowards(*ActorToCheck) != ETeamAttitude::Hostile)
	{
		return false;
	}

	return true;
}

void ATargetActor_Line::DoTargetCheckAndReport()
{
	if (!HasAuthority())
	{
		return;
	}

	// 重新计算碰撞检测参数（与UpdateTargetTrace保持一致）
	FVector ViewLocation = GetActorLocation();
	FRotator ViewRotation;
	if (AvatarActor)
	{
		AvatarActor->GetActorEyesViewPoint(ViewLocation, ViewRotation);
	}

	FVector LookEndPoint = ViewLocation + ViewRotation.Vector() * 100000.f;
	FRotator LookRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), LookEndPoint);
	FVector ForwardDirection = LookRotation.Vector();
	FVector ZOffset = FVector::UpVector * CollisionZOffset;
	FVector CollisionStartLocation = GetActorLocation() + ForwardDirection * CollisionStartOffset + ZOffset;
	FVector SweepEndLocation = CollisionStartLocation + ForwardDirection * TargetRange;

	// 收集所有碰撞结果
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(AvatarActor);
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;

	// 使用 SweepMultiByObjectType 检测多种对象类型
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	// 使用胶囊体碰撞（圆柱体形状），高度为TargetRange
	float CapsuleHalfHeight = TargetRange / 2.0f;
	TArray<FHitResult> ObjectHits;
	GetWorld()->SweepMultiByObjectType(
		ObjectHits,
		CollisionStartLocation,
		SweepEndLocation,
		FQuat::Identity,
		ObjectTypes,
		FCollisionShape::MakeCapsule(DetectionCylinderRadius, CapsuleHalfHeight),
		QueryParams
	);

	// 收集所有被击中的Actor
	TSet<AActor*> HitActorSet;
	for (const FHitResult& Hit : ObjectHits)
	{
		if (Hit.GetActor())
		{
			HitActorSet.Add(Hit.GetActor());
		}
	}

	TArray<TWeakObjectPtr<AActor>> OverlappingActors;
	for (AActor* HitActor : HitActorSet)
	{
		if (ShouldReportActorAsTarget(HitActor))
		{
			OverlappingActors.Add(HitActor);
		}
	}

	FGameplayAbilityTargetDataHandle TargetDataHandle;

	FGameplayAbilityTargetData_ActorArray* ActorArray = new FGameplayAbilityTargetData_ActorArray;
	ActorArray->SetActors(OverlappingActors);
	TargetDataHandle.Add(ActorArray);

	// 只有当有重叠的 Actor 时才广播
	if (OverlappingActors.Num() > 0)
	{
		TargetDataReadyDelegate.Broadcast(TargetDataHandle);
	}
}
