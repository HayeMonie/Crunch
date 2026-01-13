// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "UObject/NoExportTypes.h"
#include "InventoryItem.generated.h"
class UPDA_ShopItem;
class UAbilitySystemComponent;

USTRUCT()
struct FInventoryItemHandle
{
	GENERATED_BODY()

public:
	FInventoryItemHandle();
	static FInventoryItemHandle GetInvalidHandle();
	static FInventoryItemHandle CreateHandle();

	bool IsValid() const ;
	uint32 GetHandleId() const { return HandleId; }
	
private:
	explicit FInventoryItemHandle(uint32 Id);
	
	UPROPERTY()
	uint32 HandleId;

	static uint32 GenerateNextId();
	static uint32 GetInvalidId();
};

bool operator==(const FInventoryItemHandle& Lhs, const FInventoryItemHandle& Rhs);
uint32 GetTypeHash(const FInventoryItemHandle& key);



/**
 * 
 */
UCLASS()
class UInventoryItem : public UObject
{
	GENERATED_BODY()

public:
	UInventoryItem();
	bool IsValid() const;
	void InitItem(const FInventoryItemHandle& NewHandle, const UPDA_ShopItem* NewShopItem);
	const UPDA_ShopItem* GetShopItem() const { return ShopItem; }
	FInventoryItemHandle GetHandle() const { return Handle; }
	void ApplyGasModifications(UAbilitySystemComponent* AbilitySystemComponent);
	FORCEINLINE int32 GetStackCount() const { return StackCount; }
	void SetSlot(int32 NewSlot);
	
private:
	UPROPERTY()
	const UPDA_ShopItem* ShopItem;
	FInventoryItemHandle Handle;
	int32 StackCount;
	int32 Slot;

	FActiveGameplayEffectHandle AppliedEquippedEffectHandle;
	FGameplayAbilitySpecHandle GrantedAbilitySpecHandle;
};
