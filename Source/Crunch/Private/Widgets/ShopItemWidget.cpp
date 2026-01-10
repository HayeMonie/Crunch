// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/ShopItemWidget.h"
#include "Inventory/PDA_ShopItem.h"

void UShopItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	ShopItem = Cast<UPDA_ShopItem>(ListItemObject);
	if (!ShopItem)
	{
		return;
	}

	SetIcon(ShopItem->GetIcon());

	SetToolTipWidget(ShopItem);
}

void UShopItemWidget::RightButtonClicked()
{
	OnItemPurchaseIssued.Broadcast(GetShopItem());
}

void UShopItemWidget::LeftButtonClicked()
{
	OnShopItemSelected.Broadcast(this);
}
