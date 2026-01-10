// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/ItemToolTip.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Inventory/PDA_ShopItem.h"

void UItemToolTip::SetItem(const UPDA_ShopItem* Item)
{
	ItemTitleText->SetText(Item->GetItemName());
	ItemDescriptionText->SetText(Item->GetItemDescription());
	ItemPriceText->SetText(FText::AsNumber((int)Item->GetPrice()));
	IconImage->SetBrushFromTexture(Item->GetIcon());
	
}

void UItemToolTip::SetPrice(float newPrice)
{
	ItemPriceText->SetText(FText::AsNumber((int)newPrice));
}
