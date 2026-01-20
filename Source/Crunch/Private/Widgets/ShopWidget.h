// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/InventoryComponent.h"
#include "ShopWidget.generated.h"

class UItemTreeWidget;
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout", Meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float AnchorPositionRatio = 0.25f;

private:
	UPROPERTY(meta = (BindWidget))
	UTileView* ShopItemList;

	UPROPERTY(meta = (BindWidget))
	UItemTreeWidget* CombinationTree;

	void LoadShopItems();
	void ShopItemLoadFinished();
	void ShopItemWidgetGenerated(UUserWidget& NewWidget);

	UPROPERTY()
	TMap<const UPDA_ShopItem*, const UShopItemWidget*> ItemsMap;

	UPROPERTY()
	UInventoryComponent* OwnerInventoryComponent;

	void ShowItemCombination(const UShopItemWidget* ItemWidget);
};
