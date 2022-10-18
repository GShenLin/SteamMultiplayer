// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MultiplayerSessionSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSION_API UMultiplayerSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMultiplayerSessionSubsystem();

	void CreateSession(int32 NumPublicConnections, FString MatchType);
	void FindSession(int32 MaxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	void DestroySession();
	void StartSession();

protected:
	//Internal callbacks for the delegates we'll add to the Online Session Interface Delegate list
	//  These don't need to be called outside this class
	void OnCreateSessionComplete(FName SessionName , bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName , EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName , bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName , bool bWasSuccessful);
	
private:
	IOnlineSessionPtr SessionInterface;
	// To add to the Online Session Interface delegate list .
	// We'll bind our MultiplayerSessionSubsystem internal callback to these.
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteHandle;

	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteHandle;
	
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteHandle;
	
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteHandle;
	
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteHandle;
	
};
