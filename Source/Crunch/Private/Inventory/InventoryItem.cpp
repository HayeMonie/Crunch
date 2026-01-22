// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryItem.h"
#include "Inventory/PDA_ShopItem.h"
#include "AbilitySystemComponent.h"
#include "GAS/CAbilitySystemStatics.h"
#include "GAS/CAttributeSet.h"
#include "Widgets/ItemWidget.h"

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

bool UInventoryItem::IsGrantingAbility(TSubclassOf<UGameplayAbility> AbilityClass) const
{
	if (!ShopItem)
	{
		return false;
	}

	TSubclassOf<UGameplayAbility> GrantedAbility = ShopItem->GetGrantedAbility();
	return GrantedAbility == AbilityClass;
}

bool UInventoryItem::IsGrantingAnyAbility() const
{
	if (!ShopItem)
	{
		return false;
	}

	return ShopItem->GetGrantedAbility() != nullptr;
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

void UInventoryItem::InitItem(const FInventoryItemHandle& NewHandle, const UPDA_ShopItem* NewShopItem, UAbilitySystemComponent* AbilitySystemComponent)
{
	Handle = NewHandle;
	ShopItem = NewShopItem;
	OwnerAbilitySystemComponent = AbilitySystemComponent;
	if (OwnerAbilitySystemComponent)
	{
		OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetManaAttribute()).AddUObject(this, &UInventoryItem::ManaUpdated);
	}
	ApplyGASModifications();
}

bool UInventoryItem::TryActivateGrantedAbility()
{
	if (!GrantedAbilitySpecHandle.IsValid())
	{
		return false;
	}

	if (OwnerAbilitySystemComponent && OwnerAbilitySystemComponent->TryActivateAbility(GrantedAbilitySpecHandle))
	{
		return true;
	}

	return false;
}

void UInventoryItem::ApplyConsumeEffect()
{
	if (!ShopItem)
	{
		return;
	}

	TSubclassOf<UGameplayEffect> ConsumeEffect = ShopItem->GetConsumeEffect();
	if (!ConsumeEffect)
	{
		return;
	}

	OwnerAbilitySystemComponent->BP_ApplyGameplayEffectToSelf(ConsumeEffect, 1, OwnerAbilitySystemComponent->MakeEffectContext());
}

void UInventoryItem::RemoveGasModifications()
{
	if (!OwnerAbilitySystemComponent)
	{
		return;
	}

	OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetManaAttribute()).RemoveAll(this);

	if (OwnerAbilitySystemComponent->GetOwner()->HasAuthority())
	{
		if (AppliedEquippedEffectHandle.IsValid())
		{
			OwnerAbilitySystemComponent->RemoveActiveGameplayEffect(AppliedEquippedEffectHandle);
		}

		if (GrantedAbilitySpecHandle.IsValid())
		{
			OwnerAbilitySystemComponent->SetRemoveAbilityOnEnd(GrantedAbilitySpecHandle);
		}
	}
}

void UInventoryItem::ApplyGASModifications()
{
	if (!GetShopItem() || !OwnerAbilitySystemComponent)
	{
		return;
	}

	if (!OwnerAbilitySystemComponent->GetOwner() || !OwnerAbilitySystemComponent->GetOwner()->HasAuthority())
	{
		return;
	}

	TSubclassOf<UGameplayEffect> EquipEffect = GetShopItem()->GetEquippedEffect();
	if (EquipEffect)
	{
		AppliedEquippedEffectHandle = OwnerAbilitySystemComponent->BP_ApplyGameplayEffectToSelf(EquipEffect, 1 , OwnerAbilitySystemComponent->MakeEffectContext());
	}

	TSubclassOf<UGameplayAbility> GrantedAbility = GetShopItem()->GetGrantedAbility();
	if (GrantedAbility)
	{
		const FGameplayAbilitySpec* FoundSpec = OwnerAbilitySystemComponent->FindAbilitySpecFromClass(GrantedAbility);
		if (FoundSpec)
		{
			GrantedAbilitySpecHandle = FoundSpec->Handle;
		}
		else
		{
			GrantedAbilitySpecHandle = OwnerAbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(GrantedAbility));
		}
	}
}

void UInventoryItem::ManaUpdated(const FOnAttributeChangeData& ChangeData)
{
	OnAbilityCanCastUpdated.Broadcast(CanCastAbility());
}

void UInventoryItem::SetSlot(int32 NewSlot)
{
	Slot = NewSlot;
}

float UInventoryItem::GetAbilityCooldownTimeRemaining() const
{
	if (!IsGrantingAnyAbility())
	{
		return 0.f;
	}

	return UCAbilitySystemStatics::GetCooldownRemainingFor(GetShopItem()->GetGrantedAbilityCDO(), *OwnerAbilitySystemComponent);
}

float UInventoryItem::GetAbilityCooldownDuration() const
{
	if (!IsGrantingAnyAbility())
	{
		return 0.f;
	}

	return UCAbilitySystemStatics::GetCooldownDurationFor(GetShopItem()->GetGrantedAbilityCDO(), *OwnerAbilitySystemComponent, 1);
}

float UInventoryItem::GetAbilityManaCost() const
{
	if (!IsGrantingAnyAbility())
	{
		return 0.f;
	}

	return UCAbilitySystemStatics::GetManaCostFor(GetShopItem()->GetGrantedAbilityCDO(), *OwnerAbilitySystemComponent, 1);
}

bool UInventoryItem::CanCastAbility() const
{
	if (!IsGrantingAnyAbility() || !OwnerAbilitySystemComponent)
	{
		return false;
	}
	
	FGameplayAbilitySpec* Spec = OwnerAbilitySystemComponent->FindAbilitySpecFromHandle(GrantedAbilitySpecHandle);
	if (Spec)
	{
		return UCAbilitySystemStatics::CheckAbilityCost(*Spec, *OwnerAbilitySystemComponent);
	}

	return UCAbilitySystemStatics::CheckAbilityCostStatic(GetShopItem()->GetGrantedAbilityCDO(), *OwnerAbilitySystemComponent);
}
