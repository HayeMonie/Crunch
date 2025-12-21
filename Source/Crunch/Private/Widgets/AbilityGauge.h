// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "Abilities/GameplayAbility.h"
#include "AbilityGauge.generated.h"

USTRUCT(BlueprintType)
struct FAbilityWidgetData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UGameplayAbility> AbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AbilityName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;
};


/**
 * 
 */
UCLASS()
class UAbilityGauge : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	void ConfigureWithWidgetData(const FAbilityWidgetData* WidgetData);
	
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	float CooldownUpdateInterval {0.1f};

	// 是否在倒计时大于1秒时显示小数部分
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	bool bShowDecimalsWhenAboveOneSecond {false};
	
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FName IconMaterialParameterName {"Icon"};
	
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FName CooldownPercentParamName {"Percent"};
	
	UPROPERTY(meta = (BindWidget))
	class UImage* Icon;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CooldownCounterText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CooldownDurationText;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CostText;

	UPROPERTY()
	class UGameplayAbility* AbilityCDO;
	
	void AbilityCommitted(UGameplayAbility* Ability);

	void StartCooldown(float CooldownTimeRemaining, float CooldownDuration);

	float CachedCooldownDuration {0.f};
	float CachedCooldownTimeRemaining {0.f};

	FTimerHandle CooldownTimerHandle;
	FTimerHandle CooldownTimerUpdateHandle;

	FNumberFormattingOptions TwoDigitNumberFormattingOptions;
	FNumberFormattingOptions WholeNumberFormattingOptions;
	
	void CooldownFinished();
	void UpdateCooldown();
};
