// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Widgets/ItemWidget.h"
#include "ShopItemWidget.generated.h"

class UPDA_ShopItem;
/**
 * 
 */
UCLASS()
class UShopItemWidget : public UItemWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	FORCEINLINE const UPDA_ShopItem* GetShopItem() const { return ShopItem; }

private:
	UPROPERTY()
	const UPDA_ShopItem* ShopItem;
	
};
