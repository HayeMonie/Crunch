// Fill out your copyright notice in the Description page of Project Settings.

#include "FrameWork/CGameMode.h"
#include "Player/CPlayerController.h"
#include "EngineUtils.h"
#include "StormCore.h"
#include "GameFramework/PlayerStart.h"

class ACPlayerController;

APlayerController* ACGameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	APlayerController* NewPlayerController = Super::SpawnPlayerController(InRemoteRole, Options);
	IGenericTeamAgentInterface* NewPlayerTeamInterface = Cast<IGenericTeamAgentInterface>(NewPlayerController);
	FGenericTeamId TeamID = GetTeamIDForPlayer(NewPlayerController);
	if (NewPlayerTeamInterface)
	{
		NewPlayerTeamInterface->SetGenericTeamId(TeamID);
	}

	NewPlayerController->StartSpot = FindNextStartSpotForTeam(TeamID);

	return NewPlayerController;
}

void ACGameMode::StartPlay()
{
	Super::StartPlay();
	AStormCore* StormCore = GetStormCore();
	if (StormCore)
	{
		StormCore->OnGoalReachedDelegate.AddUObject(this, &ACGameMode::MatchFinished);
	}
}

FGenericTeamId ACGameMode::GetTeamIDForPlayer(const APlayerController* PlayerController) const
{
	static int PlayerCount = 0;

	++PlayerCount;
		
	return FGenericTeamId(PlayerCount % 2);
}

AActor* ACGameMode::FindNextStartSpotForTeam(const FGenericTeamId TeamID) const
{
	const FName* StartSpotTag = TeamStartSpotTagMap.Find(TeamID);

	if (!StartSpotTag)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();

	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		if (It->PlayerStartTag == *StartSpotTag)
		{
			It->PlayerStartTag = FName("Taken");
			return *It;
		}
	}

	return nullptr;
}

void ACGameMode::MatchFinished(AActor* ViewTarget, int WinningTeam)
{
	UWorld* World = GetWorld();
	if (World)
	{
		for (TActorIterator<ACPlayerController> It(World); It; ++It)
		{
			It->MatchFinished(ViewTarget, WinningTeam);
		}
	}
}

class AStormCore* ACGameMode::GetStormCore() const
{
	UWorld* World = GetWorld();
	if (World)
	{
		for (TActorIterator<AStormCore> It(World); It; ++It)
		{
			return *It;
		}
	}

	return nullptr;
}
