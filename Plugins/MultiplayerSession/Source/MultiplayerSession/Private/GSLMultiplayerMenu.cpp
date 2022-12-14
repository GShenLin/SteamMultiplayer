// Fill out your copyright notice in the Description page of Project Settings.


#include "GSLMultiplayerMenu.h"
#include "MultiplayerSessionSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Components/Button.h"

void UGSLMultiplayerMenu::MenuSetup(int NumberOfPublicConnection , FString TypeOfMatch , FString LobbyPath )
{
	LobbyListenPath = FString::Printf(TEXT("%s?listen"),*LobbyPath);
	
	NumPublicConnections = NumberOfPublicConnection;
	MatchType = TypeOfMatch;
	
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;
	
	UWorld * World = GetWorld();
	if (World)
	{
		APlayerController * PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance * GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>();
	}

	if (MultiplayerSessionSubsystem)
	{
		MultiplayerSessionSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionSubsystem->MultiplayerOnFindSessionComplete.AddUObject(this , &ThisClass::OnFindSessions);
		MultiplayerSessionSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this , &ThisClass::OnJoinSession);
		MultiplayerSessionSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this , &ThisClass::OnDestroySession);
		MultiplayerSessionSubsystem->MultplayerOnStartSessionComplete.AddDynamic(this , &ThisClass::OnStartSession);
	}
}

void UGSLMultiplayerMenu::OnCreateSession(bool bWasSuccessful)
{
	UE_LOG(LogTemp , Warning , TEXT("CreateSessionCalled"));
	if (bWasSuccessful)
	{
		UWorld * World = GetWorld();
		if (IsValid(World))
		{
			World->ServerTravel(LobbyListenPath);
		}
		else
		{
			UE_LOG(LogTemp , Warning , TEXT("World NULL  : Menu"));
			HostButton->SetIsEnabled(true);
		}
	}
	else
	{
		UE_LOG(LogTemp , Warning , TEXT("CreateSessionFail : Menu"));
		HostButton->SetIsEnabled(true);
	}
}

void UGSLMultiplayerMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResult, bool bWasSuccessful)
{
	if (MultiplayerSessionSubsystem == nullptr)
	{
		return;
	}
	for (auto Result : SessionResult)
	{
		//????????????????????????
		FString SettingValue;
		Result.Session.SessionSettings.Get(FName("MatchType")  , SettingValue);
		if (SettingValue == MatchType )
		{
			MultiplayerSessionSubsystem->JoinSession(Result);
			UE_LOG(LogTemp , Warning , TEXT("Joining Session"));
			return;
		}
	}
	UE_LOG(LogTemp , Warning , TEXT("Join Session Failure , No Matching Session : Menu"));
	if (bWasSuccessful || SessionResult.Num() == 0)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UGSLMultiplayerMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		IOnlineSessionPtr SessionInterface =  OnlineSubsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FString Address;
			bool bJoinSuccessful = SessionInterface->GetResolvedConnectString(NAME_GameSession,Address);
			if (bJoinSuccessful)
			{
				UE_LOG(LogTemp , Warning , TEXT("JoinSuccessful : %s"), *Address);
				APlayerController * PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
				if (PlayerController)
				{
					PlayerController->ClientTravel(Address,ETravelType::TRAVEL_Absolute);
				}
				else
				{
					UE_LOG(LogTemp , Warning , TEXT("Player Controller is NULL : Menu"), *Address);
				}
			}
		}
	}
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UGSLMultiplayerMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UGSLMultiplayerMenu::OnStartSession(bool bWasSuccessful)
{
}

bool UGSLMultiplayerMenu::Initialize()
{
	 if(!Super::Initialize())
	 {
	 	return false;
	 }
	NumPublicConnections = 4;
	MatchType = TEXT("FreeForAll");
	
	 if (HostButton)
	 {
		 HostButton->OnClicked.AddDynamic(this,&ThisClass::HostButtonClicked);
	 }
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this,&ThisClass::JoinButtonClicked);
	}
	return true;
}

void UGSLMultiplayerMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	MenuTearDown();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UGSLMultiplayerMenu::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	if (MultiplayerSessionSubsystem)
	{
		bool bCreateSuccessful = MultiplayerSessionSubsystem->CreateSession(NumPublicConnections,MatchType);
	}
}

void UGSLMultiplayerMenu::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	if (MultiplayerSessionSubsystem)
	{
		//???????????????????????????????????????SteamAppID ??????????????????
		MultiplayerSessionSubsystem->FindSession(10000);
		UE_LOG(LogTemp , Warning , TEXT("Buttion Clicked , Finding Session"));
	}
}

void UGSLMultiplayerMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController * PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}
