// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Engine/DataAsset.h"
#include "PDA_ShopItem.generated.h"


class UPDA_ShopItem;

USTRUCT(Blueprintable)
struct FItemCollection
{
	GENERATED_BODY()

public:
	FItemCollection();
	FItemCollection(const TArray<const UPDA_ShopItem*>& InItems);
	void AddItem(const UPDA_ShopItem* Item, bool bAddUnique = false);
	bool Contains(const UPDA_ShopItem* Item) const;
	const TArray<const UPDA_ShopItem*>& GetItems() const;
	
private:
	TArray<const UPDA_ShopItem*> Items;
};

/**
 * 
 */
UCLASS()
class UPDA_ShopItem : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	static FPrimaryAssetType GetShopItemAssetType();
	UTexture2D* GetIcon() const;
	FText GetItemName() const { return ItemName; }
	FText GetItemDescription() const { return ItemDescription; }
	float GetPrice() const { return Price; }
	float GetSellPrice() const { return Price * 0.65f; }
	
	TSubclassOf<class UGameplayEffect> GetEquippedEffect() const { return EquippedEffect; }
	TSubclassOf<class UGameplayEffect> GetConsumeEffect() const { return ConsumeEffect; }
	TSubclassOf<class UGameplayAbility> GetGrantedAbility() const { return GrantedAbility; }
	bool IsStackable() const { return bIsStackable; }
	bool GetIsConsumable() const { return bIsConsumable; }
	int32 GetMaxStackCount() const { return MaxStackCount; }
	const TArray<TSoftObjectPtr<UPDA_ShopItem>>& GetIngredientItems() const { return IngredientItems; }


private:
	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	TSoftObjectPtr<class UTexture2D> Icon;
	
	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	float Price;
	
	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	FText ItemName;

	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	FText ItemDescription;
	
	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	bool bIsConsumable;

	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	TSubclassOf<class UGameplayEffect> EquippedEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	TSubclassOf<class UGameplayEffect> ConsumeEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	TSubclassOf<class UGameplayAbility> GrantedAbility;

	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	bool bIsStackable {false};

	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	int32 MaxStackCount {5};

	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	TArray<TSoftObjectPtr<UPDA_ShopItem>> IngredientItems;
	
};
