// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "GSLMultiplayerMenu.generated.h"

UCLASS()
class MULTIPLAYERSESSION_API UGSLMultiplayerMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int NumberOfPublicConnection = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")) , FString LobbyPath = "");

	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);

	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResult , bool bWasSuccessful );

	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);
	
protected:
	//比较好的绑定回调的地方
	virtual bool Initialize() override;
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;
	
private:
	UPROPERTY(meta=(BindWidget))
	class UButton* HostButton;
	
	UPROPERTY(meta=(BindWidget))
	UButton* JoinButton;

	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();

	void MenuTearDown();
	
	class UMultiplayerSessionSubsystem * MultiplayerSessionSubsystem;
	int  NumPublicConnections;
	FString MatchType;
	FString LobbyListenPath;
	
};
