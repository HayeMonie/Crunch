// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CGameplayAbility.h"
#include "Components/SkeletalMeshComponent.h"

class UAnimInstance* UCGameplayAbility::GetOwnerAnimInstance() const
{
	USkeletalMeshComponent* OwnerSkeletalMeshComp = GetOwningComponentFromActorInfo();
	if (OwnerSkeletalMeshComp)
	{
		return OwnerSkeletalMeshComp->GetAnimInstance();
	}

	return nullptr;
}
