// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	//获取玩家数量
	if (GameState)
	{
		int32 NumberOfPlayer = GameState.Get()->PlayerArray.Num();
	}

	//PlayerController中拿到PlayerState
	APlayerState * PlayerState = NewPlayer->GetPlayerState<APlayerState>();
	if (PlayerState)
	{
		FString PlayerName = PlayerState->GetPlayerName();
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	//获取玩家数量
	if (GameState)
	{
		int32 NumberOfPlayer = GameState.Get()->PlayerArray.Num() -1; // 离开时还没离开 -1才是离开后的玩家数量
	}

	//PlayerController中拿到PlayerState
	APlayerState * PlayerState = Exiting->GetPlayerState<APlayerState>();
	if (PlayerState)
	{
		FString PlayerName = PlayerState->GetPlayerName();
	}
}
