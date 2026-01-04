// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "Inventory/PDA_ShopItem.h"
#include "CAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class UCAssetManager : public UAssetManager
{
	GENERATED_BODY()
public:
	static UCAssetManager& Get();
	void LoadShopItems(const FStreamableDelegate& LoadFinishedCallback);
	bool GetLoadedShopItems(TArray<const UPDA_ShopItem*>& OutItems) const;

private:
	void ShopItemLoadFinished(FStreamableDelegate Callback);
	void BuildItemMap();
	void AddToCombinationMap(const UPDA_ShopItem* Ingredient, const UPDA_ShopItem* CombinationItem);

	UPROPERTY()
	TMap<const UPDA_ShopItem*, FItemCollection> CombinationMap;

	UPROPERTY()
	TMap<const UPDA_ShopItem*, FItemCollection> IngredientMap;
	
};
