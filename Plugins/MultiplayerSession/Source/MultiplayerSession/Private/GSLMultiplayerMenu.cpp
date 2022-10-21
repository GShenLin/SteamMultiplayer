// Fill out your copyright notice in the Description page of Project Settings.


#include "GSLMultiplayerMenu.h"
#include "MultiplayerSessionSubsystem.h"
#include "Components/Button.h"

void UGSLMultiplayerMenu::MenuSetup(int NumberOfPublicConnection , FString TypeOfMatch )
{
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
	UE_LOG(LogTemp , Warning , TEXT("Host ButtonClicked"));
	if (MultiplayerSessionSubsystem)
	{
		bool bCreateSuccessful = MultiplayerSessionSubsystem->CreateSession(NumPublicConnections,MatchType);
		UWorld * World = GetWorld();
		if (bCreateSuccessful  && IsValid(World))
		{
			World->ServerTravel("/Game/Level/Lobby");
		}
		else
		{
			UE_LOG(LogTemp , Warning , TEXT("CreateSessionFail  OnMenuClicked")); 
		}
	}
	
}

void UGSLMultiplayerMenu::JoinButtonClicked()
{
	UE_LOG(LogTemp , Warning , TEXT("Join ButtonClicked"));
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
