// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/TreeNodeInterface.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Components/ListView.h"
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
class UShopItemWidget : public UItemWidget, public IUserObjectListEntry, public ITreeNodeInterface
{
	GENERATED_BODY()

public:
	FOnItemPurchaseIssued OnItemPurchaseIssued;
	FOnShopItemSelected OnShopItemClicked;

	virtual UUserWidget* GetWidget() const override;
	virtual TArray<const ITreeNodeInterface*> GetInputs() const override;
	virtual TArray<const ITreeNodeInterface*> GetOutputs() const override;
	virtual const UObject* GetItemObject() const override;
	
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	FORCEINLINE const UPDA_ShopItem* GetShopItem() const { return ShopItem; }

private:
	void CopyFromOther(const UShopItemWidget* OtherWidget);
	void InitWithShopItem(const UPDA_ShopItem* NewShopItem);
	TArray<const ITreeNodeInterface*> ItemsToInterfaces(const TArray<const UPDA_ShopItem*>& Items) const;
	
	UPROPERTY()
	const UPDA_ShopItem* ShopItem;

	UPROPERTY(Transient)
	const class UListView* ParentListView;
	
	virtual void RightButtonClicked() override;
	virtual void LeftButtonClicked() override;
	
};
