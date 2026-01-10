// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryComponent.h"
#include "Inventory/PDA_ShopItem.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/CHeroAttributeSet.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UInventoryComponent::TryPurchase(const UPDA_ShopItem* ItemToPurchase)
{
	if (!OwnerAbilitySystemComponent)
	{
		return;
	}

	if (!ItemToPurchase)
	{
		return;
	}

	Server_Purchase(ItemToPurchase);
}

float UInventoryComponent::GetGold() const
{
	bool bFound = false;
	if (OwnerAbilitySystemComponent)
	{
		float Gold = OwnerAbilitySystemComponent->GetGameplayAttributeValue(UCHeroAttributeSet::GetGoldAttribute(), bFound);
		if (bFound)
		{
			return Gold;
		}
	}

	return 0.f;
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	OwnerAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());	
}

void UInventoryComponent::Server_Purchase_Implementation(const UPDA_ShopItem* ItemToPurchase)
{
	if (!ItemToPurchase)
	{
		return;
	}

	if (GetGold() < ItemToPurchase->GetPrice())
	{
		return;
	}

	OwnerAbilitySystemComponent->ApplyModToAttribute(UCHeroAttributeSet::GetGoldAttribute(), EGameplayModOp::Additive, -ItemToPurchase->GetPrice());
	UE_LOG(LogTemp, Warning, TEXT("Item Name : %s"), *ItemToPurchase->GetName());
}

bool UInventoryComponent::Server_Purchase_Validate(const UPDA_ShopItem* ItemToPurchase)
{
	return true;
}


