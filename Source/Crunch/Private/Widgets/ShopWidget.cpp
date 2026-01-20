// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/ShopWidget.h"
#include "Inventory/InventoryComponent.h"
#include "ShopItemWidget.h"
#include "ItemTreeWidget.h"
#include "Components/TileView.h"
#include "Components/CanvasPanelSlot.h"
#include "FrameWork/CAssetManager.h"

void UShopWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
	
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
	{
		CanvasSlot->SetAnchors(FAnchors(AnchorPositionRatio));
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasSlot->SetPosition(FVector2D::ZeroVector);
	}
	
	LoadShopItems();
	ShopItemList->OnEntryWidgetGenerated().AddUObject(this, &UShopWidget::ShopItemWidgetGenerated);
	if (APawn* OwnerPawn = GetOwningPlayerPawn())
	{
		OwnerInventoryComponent = OwnerPawn->GetComponentByClass<UInventoryComponent>();
	}
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
		if (OwnerInventoryComponent)
		{
			ItemWidget->OnItemPurchaseIssued.AddUObject(OwnerInventoryComponent, &UInventoryComponent::TryPurchase);
		}
		
		ItemWidget->OnShopItemClicked.AddUObject(this, &UShopWidget::ShowItemCombination);

		ItemsMap.Add(ItemWidget->GetShopItem(), ItemWidget);
	}
}

void UShopWidget::ShowItemCombination(const UShopItemWidget* ItemWidget)
{
	if (CombinationTree && ItemWidget)
	{
		CombinationTree->DrawFromNode(ItemWidget);
	}
}
