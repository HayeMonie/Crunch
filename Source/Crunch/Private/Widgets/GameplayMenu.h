// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "GameplayMenu.generated.h"

/**
 * 
 */
UCLASS()
class UGameplayMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	FOnButtonClickedEvent& GetResumeButtonClickedEventDelegate();
	void SetTitleText(const FString& NewTitle);

private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MenuTitle;

	UPROPERTY(meta = (BindWidget))
	class UButton* ResumeButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* MainMenuButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* QuitGameButton;

	UFUNCTION()
	void BackToMainMenu();

	UFUNCTION()
	void QuitGame();
};


