// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/MatchStatWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "FrameWork/StormCore.h"
#include "Kismet/GameplayStatics.h"

void UMatchStatWidget::NativeConstruct()
{
	Super::NativeConstruct();
	StormCore = Cast<AStormCore>(UGameplayStatics::GetActorOfClass(this, AStormCore::StaticClass()));
	if (StormCore)
	{
		StormCore->OnTeamInfluencerCountUpdatedDelegate.AddUObject(this, &UMatchStatWidget::UpdateTeamInfluence);
		StormCore->OnGoalReachedDelegate.AddUObject(this, &UMatchStatWidget::MatchFinished);
	}
}

void UMatchStatWidget::UpdateTeamInfluence(int32 TeamOneCount, int32 TeamTwoCount)
{
	TeamOneCountText->SetText(FText::AsNumber(TeamOneCount));
	TeamTwoCountText->SetText(FText::AsNumber(TeamTwoCount));
}

void UMatchStatWidget::MatchFinished(AActor* ViewTarget, int WinningTeam)
{
	
}
