// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Widgets/ItemWidget.h"
#include "ShopItemWidget.generated.h"

class UShopItemWidget;
class UPDA_ShopItem;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemPurchaseIssued, const UPDA_ShopItem*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnShopItemSelected, const UShopItemWidget*);
/**
 * 
 */
UCLASS()
class UShopItemWidget : public UItemWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	FOnItemPurchaseIssued OnItemPurchaseIssued;
	FOnShopItemSelected OnShopItemSelected;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	FORCEINLINE const UPDA_ShopItem* GetShopItem() const { return ShopItem; }

private:
	UPROPERTY()
	const UPDA_ShopItem* ShopItem;
	
	virtual void RightButtonClicked() override;
	virtual void LeftButtonClicked() override;
	
};
