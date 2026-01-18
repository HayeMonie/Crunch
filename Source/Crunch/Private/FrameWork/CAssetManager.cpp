// Fill out your copyright notice in the Description page of Project Settings.


#include "FrameWork/CAssetManager.h"

UCAssetManager& UCAssetManager::Get()
{
	UCAssetManager* Singleton = Cast<UCAssetManager>(GEngine->AssetManager.Get());
	if (Singleton)
	{
		return *Singleton;
	}
	UE_LOG(LogLoad, Fatal, TEXT("Asset Manager Needs to be as the type CAssetManager"));
	return *NewObject<UCAssetManager>();
}

void UCAssetManager::LoadShopItems(const FStreamableDelegate& LoadFinishedCallback)
{
	LoadPrimaryAssetsWithType(UPDA_ShopItem::GetShopItemAssetType(), TArray<FName>(), FStreamableDelegate::CreateUObject(this, &UCAssetManager::ShopItemLoadFinished, LoadFinishedCallback));
}

bool UCAssetManager::GetLoadedShopItems(TArray<const UPDA_ShopItem*>& OutItems) const
{
	TArray<UObject*> LoadedObjects;
	bool bLoaded = GetPrimaryAssetObjectList(UPDA_ShopItem::GetShopItemAssetType(), LoadedObjects);

	if (bLoaded)
	{
		for (UObject* ObjectLoaded : LoadedObjects)
		{
			OutItems.Add(Cast<UPDA_ShopItem>(ObjectLoaded));
		}
	}

	return bLoaded;
}

const FItemCollection* UCAssetManager::GetCombinationForItem(const UPDA_ShopItem* Item) const
{
	return CombinationMap.Find(Item);
}

const FItemCollection* UCAssetManager::GetIngredientForItem(const UPDA_ShopItem* Item) const
{
	return IngredientMap.Find(Item);
}

void UCAssetManager::ShopItemLoadFinished(FStreamableDelegate Callback)
{
	Callback.ExecuteIfBound();
	BuildItemMap();
}

void UCAssetManager::BuildItemMap()
{
	TArray<const UPDA_ShopItem*> LoadedItems;
	if (GetLoadedShopItems(LoadedItems))
	{
		for (const UPDA_ShopItem* Item : LoadedItems)
		{
			if (Item->GetIngredientItems().Num() == 0)
			{
				continue;
			}

			TArray<const  UPDA_ShopItem*> Items;
			for (const TSoftObjectPtr<UPDA_ShopItem>& Ingredient : Item->GetIngredientItems())
			{
				UPDA_ShopItem* IngredientItem = Ingredient.LoadSynchronous();
				Items.Add(IngredientItem);
				AddToCombinationMap(IngredientItem, Item);
			}

			IngredientMap.Add(Item,  FItemCollection{Items});
		}
	}
}

void UCAssetManager::AddToCombinationMap(const UPDA_ShopItem* Ingredient, const UPDA_ShopItem* CombinationItem)
{
	FItemCollection* Combinations = CombinationMap.Find(Ingredient);
	if (Combinations)
	{
		if (! Combinations->Contains(CombinationItem))
		{
			CombinationMap.Add(CombinationItem);
		}
	}
	else
	{
		CombinationMap.Add(Ingredient, FItemCollection{TArray<const UPDA_ShopItem*>{CombinationItem}});
	}
}
