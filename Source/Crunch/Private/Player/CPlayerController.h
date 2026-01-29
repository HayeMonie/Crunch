// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "Widgets/GameplayWidget.h"
#include "CPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ACPlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// Only Called On Server
	void OnPossess(APawn* NewPawn) override;
	
	// Only Called On Client, Also On the Listening Server.
	void AcknowledgePossession(APawn* NewPawn) override;

	/** Assigns Team Agent to given TeamID */
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	
	/** Retrieve team identifier in form of FGenericTeamId */
	virtual FGenericTeamId GetGenericTeamId() const override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupInputComponent() override;
	void MatchFinished(AActor* ViewTarget, int WinningTeam);
	
private:
	UFUNCTION(Client, Reliable)
	void Client_MatchFinished(AActor* ViewTarget, int WinningTeam);

	void SpawnGameplayWidget();

	UPROPERTY(EditDefaultsOnly, Category = "View")
	float MatchFinishedViewBlendTimeDuration = 2.f;
	
	UPROPERTY()
	class ACPlayerCharacter* CPlayerCharacter;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UGameplayWidget> GameplayWidgetClass;

	UPROPERTY()
	class UGameplayWidget* GameplayWidget;

	UPROPERTY(Replicated)
	FGenericTeamId TeamID;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* UIInputMapping;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ShopToggleInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ToggleGameplayMenuAction;

	UFUNCTION()
	void ToggleShop();

	UFUNCTION()
	void ToggleGameplayMenu();

	void ShowWinLoseState();
};
