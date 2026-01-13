// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InventoryItemWidget.h"

#include "ItemToolTip.h"
#include "Inventory/InventoryItem.h"
#include "Components/TextBlock.h"
#include "Inventory/PDA_ShopItem.h"

void UInventoryItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
	EmptySlot();
}

bool UInventoryItemWidget::IsEmpty() const
{
	return !InventoryItem || !InventoryItem->IsValid();
}

void UInventoryItemWidget::UpdateInventoryItem(const UInventoryItem* Item)
{
	InventoryItem = Item;
	if (!InventoryItem || !InventoryItem->IsValid() || InventoryItem->GetStackCount() <= 0 )
	{
		EmptySlot();
	}

	SetIcon(Item->GetShopItem()->GetIcon());
	UItemToolTip* ToolTip = SetToolTipWidget(InventoryItem->GetShopItem());
	if (ToolTip)
	{
		ToolTip->SetPrice(InventoryItem->GetShopItem()->GetSellPrice());
	}

	if (InventoryItem->GetShopItem()->IsStackable())
	{
		StackCountText->SetVisibility(ESlateVisibility::Visible);
		UpdateStackCount();
	}
	else
	{
		StackCountText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UInventoryItemWidget::EmptySlot()
{
	InventoryItem = nullptr;
	SetIcon(EmptyTexture);
	SetToolTip(nullptr);

	StackCountText->SetVisibility(ESlateVisibility::Hidden);
	ManaCostText->SetVisibility(ESlateVisibility::Hidden);
	CooldownCountText->SetVisibility(ESlateVisibility::Hidden);
	CooldownDurationText->SetVisibility(ESlateVisibility::Hidden);
}

void UInventoryItemWidget::UpdateStackCount()
{
	if (InventoryItem)
	{
		StackCountText->SetText(FText::AsNumber(InventoryItem->GetStackCount()));
	}
}

