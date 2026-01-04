// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "CGameplayAbilityTypes.generated.h"

UENUM(BlueprintType)
enum class ECAbilityInputID : uint8
{
    None                            UMETA(DisplayName = "None"),
    BasicAttack                     UMETA(DisplayName = "Basic Attack"),
    AbilityOne                      UMETA(DisplayName = "Ability One"),
    AbilityTwo                      UMETA(DisplayName = "Ability Two"),
    AbilityThree                    UMETA(DisplayName = "Ability Three"),
    AbilityFour                     UMETA(DisplayName = "Ability Four"),
    AbilityFive                     UMETA(DisplayName = "Ability Five"),
    AbilitySix                      UMETA(DisplayName = "Ability Six"),
    Confirm                         UMETA(DisplayName = "Confirm"),
    Cancel                          UMETA(DisplayName = "Cancel"),

};

USTRUCT(Blueprintable)
struct FGenericDamageEffectDef
{
    GENERATED_BODY()

public:
    FGenericDamageEffectDef();
    
    UPROPERTY(EditAnywhere)
    TSubclassOf<UGameplayEffect> DamageEffect;

    UPROPERTY(EditAnywhere)
    FVector PushVelocity{FVector::ZeroVector};
};

USTRUCT(Blueprintable)
struct FHeroBaseStats : public FTableRowBase
{
    GENERATED_BODY()

public:
    FHeroBaseStats();
    
    UPROPERTY(EditAnywhere)
    TSubclassOf<AActor> Class;
    
    UPROPERTY(EditAnywhere)
    float Strength{0.f};
    
    UPROPERTY(EditAnywhere)
    float Intelligence{0.f};

    UPROPERTY(EditAnywhere)
    float StrengthGrowthRate{0.f};

    UPROPERTY(EditAnywhere)
    float IntelligenceGrowthRate{0.f};

    UPROPERTY(EditAnywhere)
    float BaseMaxHealth{0.f};

    UPROPERTY(EditAnywhere)
    float BaseMaxMana{0.f};

    UPROPERTY(EditAnywhere)
    float BaseAttackDamage{0.f};

    UPROPERTY(EditAnywhere)
    float BaseArmor{0.f};

    UPROPERTY(EditAnywhere)
    float BaseMoveSpeed{0.f};
    
};


