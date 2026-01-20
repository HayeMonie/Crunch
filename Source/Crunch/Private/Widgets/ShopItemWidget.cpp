// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/ShopItemWidget.h"
#include "Inventory/PDA_ShopItem.h"
#include "Components/ListView.h"
#include "FrameWork/CAssetManager.h"

UUserWidget* UShopItemWidget::GetWidget() const
{
	UShopItemWidget* Copy = CreateWidget<UShopItemWidget>(GetOwningPlayer(), GetClass());
	Copy->CopyFromOther(this);
	return Copy;
}

TArray<const ITreeNodeInterface*> UShopItemWidget::GetInputs() const
{
	const FItemCollection* Collection = UCAssetManager::Get().GetCombinationForItem(GetShopItem());
	if (Collection)
	{
		return ItemsToInterfaces(Collection->GetItems());
	}
	
	return TArray<const ITreeNodeInterface*>();
}

TArray<const ITreeNodeInterface*> UShopItemWidget::GetOutputs() const
{
	const FItemCollection* Collection = UCAssetManager::Get().GetIngredientForItem(GetShopItem());
	if (Collection)
	{
		return ItemsToInterfaces(Collection->GetItems());
	}
	return TArray<const ITreeNodeInterface*>();
}

const UObject* UShopItemWidget::GetItemObject() const
{
	return ShopItem;
}

void UShopItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	InitWithShopItem(Cast<UPDA_ShopItem>(ListItemObject));
	ParentListView = Cast<UListView>(IUserListEntry::GetOwningListView());
}

void UShopItemWidget::CopyFromOther(const UShopItemWidget* OtherWidget)
{
	OnItemPurchaseIssued = OtherWidget->OnItemPurchaseIssued;
	OnShopItemClicked = OtherWidget->OnShopItemClicked;
	ParentListView = OtherWidget->ParentListView;
	InitWithShopItem(OtherWidget->GetShopItem());
}

void UShopItemWidget::InitWithShopItem(const UPDA_ShopItem* NewShopItem)
{
	ShopItem = NewShopItem;
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
	OnShopItemClicked.Broadcast(this);
}

TArray<const ITreeNodeInterface*> UShopItemWidget::ItemsToInterfaces(const TArray<const UPDA_ShopItem*>& Items) const
{
	TArray<const ITreeNodeInterface*> ReturnInterfaces;
	if (!ParentListView)
	{
		return ReturnInterfaces;
	}

	for (const UPDA_ShopItem* Item : Items)
	{
		const UShopItemWidget* ItemWidget = ParentListView->GetEntryWidgetFromItem<UShopItemWidget>(Item);
		if (ItemWidget)
		{
			ReturnInterfaces.Add(ItemWidget);
		}
	}

	return ReturnInterfaces;
}
