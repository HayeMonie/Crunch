// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/InventoryItem.h"
#include "InventoryComponent.generated.h"

class UAbilitySystemComponent;
class UPDA_ShopItem;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemAddedDelegate, const UInventoryItem* /*NewItem*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemRemovedDelegate, const FInventoryItemHandle& /*ItemHandle*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemStackCountChangeDelegate, const FInventoryItemHandle&, int  /*NewItem*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnItemAbilityCommittedDelegate, const FInventoryItemHandle&, float /*CooldownDuration*/, float /*CooldownTimeRemaining*/)

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();
	FOnItemAddedDelegate OnItemAdded;
	FOnItemStackCountChangeDelegate OnItemStackCountChange;
	FOnItemRemovedDelegate OnItemRemoved;
	FOnItemAbilityCommittedDelegate OnItemAbilityCommitted;

	void TryActivateItem(const FInventoryItemHandle& ItemHandle);
	void TryPurchase(const UPDA_ShopItem* ItemToPurchase);
	void SellItem(const FInventoryItemHandle& ItemHandle);
	float GetGold() const;
	FORCEINLINE int GetCapacity() const { return Capacity; }

	void ItemSlotChanged(const FInventoryItemHandle& Handle, int NewSlotNumber);
	UInventoryItem* GetInventoryItemByHandle(const FInventoryItemHandle& Handle) const;

	bool IsFullFor(const UPDA_ShopItem* Item) const;
	
	bool IsAllSlotOccupied() const;
	UInventoryItem* GetAvailableStackForItem(const UPDA_ShopItem* Item) const;
	bool FindIngredientForItem(const UPDA_ShopItem* Item, TArray<UInventoryItem*>& OutIngredients, const TArray<const UPDA_ShopItem*>& IngredientToIgnore = TArray<const UPDA_ShopItem*>{}) const;
	UInventoryItem* TryGetItemForShopItem(const UPDA_ShopItem* Item) const;
	void TryActivateItemInSlot(int SlotNumber);
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int Capacity { 6 };
	
	UPROPERTY()
	UAbilitySystemComponent* OwnerAbilitySystemComponent;

	UPROPERTY()
	TMap<FInventoryItemHandle, UInventoryItem*> InventoryMap;

	void AbilityCommitted(class UGameplayAbility* CommittedAbility);

	/********************************************************************************/
	/*                                  Server                                      */
	/********************************************************************************/
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Purchase(const UPDA_ShopItem* ItemToPurchase);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ActivateItem(const FInventoryItemHandle ItemHandle);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SellItem(const FInventoryItemHandle ItemHandle);
	
	void GrantItem(const UPDA_ShopItem* NewItem);
	void ConsumeItem(UInventoryItem* Item);
	void RemoveItem(UInventoryItem* Item);
	bool TryItemCombination(const UPDA_ShopItem* NewItem);

	/********************************************************************************/
	/*                                  Client                                      */
	/********************************************************************************/
private:
	UFUNCTION(Client, Reliable)
	void Client_ItemAdded(FInventoryItemHandle AssignedHandle, const UPDA_ShopItem* Item, FGameplayAbilitySpecHandle GrantedAbilitySpecHandle);

	UFUNCTION(Client, Reliable)
	void Client_ItemRemoved(FInventoryItemHandle ItemHandle);

	UFUNCTION(Client, Reliable)
	void Client_ItemStackCountChanged(FInventoryItemHandle Handle, int NewCount);
	
};


