// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopWidget.generated.h"

class UPDA_ShopItem;
class UShopItemWidget;
class UTileView;
/**
 * 
 */
UCLASS()
class UShopWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	UTileView* ShopItemList;

	void LoadShopItems();
	void ShopItemLoadFinished();
	void ShopItemWidgetGenerated(UUserWidget& NewWidget);

	UPROPERTY()
	TMap<const UPDA_ShopItem*, const UShopItemWidget*> ItemsMap;
	
};
