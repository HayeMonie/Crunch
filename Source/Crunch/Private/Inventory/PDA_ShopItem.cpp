// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/PDA_ShopItem.h"

FItemCollection::FItemCollection() : Items{}
{
}

FItemCollection::FItemCollection(const TArray<const UPDA_ShopItem*>& InItems) : Items{InItems}
{
}

void FItemCollection::AddItem(const UPDA_ShopItem* Item, bool bAddUnique)
{
	if (bAddUnique && Contains(Item))
	{
		return;
	}
	Items.Add(Item);
}

bool FItemCollection::Contains(const UPDA_ShopItem* Item) const
{
	return Items.Contains(Item);
}

const TArray<const UPDA_ShopItem*>& FItemCollection::GetItems() const
{
	return Items;
}

FPrimaryAssetId UPDA_ShopItem::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(GetShopItemAssetType(), GetFName());
}

FPrimaryAssetType UPDA_ShopItem::GetShopItemAssetType()
{
	return FPrimaryAssetType("ShopItem");
}

UTexture2D* UPDA_ShopItem::GetIcon() const
{
	return Icon.LoadSynchronous();
}
