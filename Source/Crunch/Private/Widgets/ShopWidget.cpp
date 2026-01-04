// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/ShopWidget.h"

#include "ShopItemWidget.h"
#include "Components/TileView.h"
#include "FrameWork/CAssetManager.h"

void UShopWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
	LoadShopItems();
	ShopItemList->OnEntryWidgetGenerated().AddUObject(this, &UShopWidget::ShopItemWidgetGenerated);
}

void UShopWidget::LoadShopItems()
{
	UCAssetManager::Get().LoadShopItems(FStreamableDelegate::CreateUObject(this, &UShopWidget::ShopItemLoadFinished));
}

void UShopWidget::ShopItemLoadFinished()
{
	TArray<const UPDA_ShopItem*> ShopItems;
	UCAssetManager::Get().GetLoadedShopItems(ShopItems);
	for (const UPDA_ShopItem* ShopItem : ShopItems)
	{
		ShopItemList->AddItem(const_cast<UPDA_ShopItem*>(ShopItem));
	}
}

void UShopWidget::ShopItemWidgetGenerated(UUserWidget& NewWidget)
{
	UShopItemWidget* ItemWidget = Cast<UShopItemWidget>(&NewWidget);
	if (ItemWidget)
	{
		ItemsMap.Add(ItemWidget->GetShopItem(), ItemWidget);
	}
}
