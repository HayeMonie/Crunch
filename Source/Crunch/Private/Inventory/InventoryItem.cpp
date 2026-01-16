// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryItem.h"
#include "Inventory/PDA_ShopItem.h"
#include "AbilitySystemComponent.h"

FInventoryItemHandle::FInventoryItemHandle() : HandleId{ GetInvalidId() }
{
	
}

FInventoryItemHandle FInventoryItemHandle::GetInvalidHandle()
{
	static FInventoryItemHandle InvalidHandle = FInventoryItemHandle();
	return InvalidHandle;
}

FInventoryItemHandle::FInventoryItemHandle(uint32 Id) : HandleId{ Id }
{
	
}

FInventoryItemHandle FInventoryItemHandle::CreateHandle()
{
	return FInventoryItemHandle{ GenerateNextId() };
}

bool FInventoryItemHandle::IsValid() const
{
	return HandleId != GetInvalidId();
}

uint32 FInventoryItemHandle::GenerateNextId()
{
	static uint32 StaticId = 1;
	return ++StaticId;
}

uint32 FInventoryItemHandle::GetInvalidId()
{
	return 0;
}

uint32 GetTypeHash(const FInventoryItemHandle& key)
{
	return key.GetHandleId();
}

bool UInventoryItem::AddStackCount()
{
	if (IsStackFull())
	{
		return false;
	}

	++StackCount;
	return true;
}

bool UInventoryItem::ReduceStackCount()
{
	--StackCount;
	if (StackCount <= 0)
	{
		return false;
	}

	return true;
}

bool UInventoryItem::SetStackCount(int32 NewStackCount)
{
	if (NewStackCount > 0 && NewStackCount <= GetShopItem()->GetMaxStackCount())
	{
		StackCount = NewStackCount;
		return true;
	}

	return false;
}

bool UInventoryItem::IsStackFull() const
{
	return StackCount >= GetShopItem()->GetMaxStackCount();
}

bool UInventoryItem::IsForItem(const UPDA_ShopItem* Item) const
{
	if (!Item)
	{
		return false;
	}

	return GetShopItem() == Item;
}

UInventoryItem::UInventoryItem() : StackCount{ 1 }
{
	
}

bool UInventoryItem::IsValid() const
{
	return ShopItem != nullptr;
}

bool operator==(const FInventoryItemHandle& Lhs, const FInventoryItemHandle& Rhs)
{
	return Lhs.GetHandleId() == Rhs.GetHandleId();
}

void UInventoryItem::InitItem(const FInventoryItemHandle& NewHandle, const UPDA_ShopItem* NewShopItem)
{
	Handle = NewHandle;
	ShopItem = NewShopItem;
}

void UInventoryItem::ApplyGasModifications(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (!GetShopItem() || !AbilitySystemComponent)
	{
		return;
	}

	if (!AbilitySystemComponent->GetOwner() || !AbilitySystemComponent->GetOwner()->HasAuthority())
	{
		return;
	}

	TSubclassOf<UGameplayEffect> EquipEffect = GetShopItem()->GetEquippedEffect();
	if (EquipEffect)
	{
		AppliedEquippedEffectHandle = AbilitySystemComponent->BP_ApplyGameplayEffectToSelf(EquipEffect, 1 , AbilitySystemComponent->MakeEffectContext());
	}

	TSubclassOf<UGameplayAbility> GrantedAbility = GetShopItem()->GetGrantedAbility();
	if (GrantedAbility)
	{
		const FGameplayAbilitySpec* FoundSpec = AbilitySystemComponent->FindAbilitySpecFromClass(GrantedAbility);
		if (FoundSpec)
		{
			GrantedAbilitySpecHandle = FoundSpec->Handle;
		}
		else
		{
			GrantedAbilitySpecHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(GrantedAbility));
		}
	}
}

void UInventoryItem::SetSlot(int32 NewSlot)
{
	Slot = NewSlot;
}
