// Fill out your copyright notice in the Description page of Project Settings.


#include "GSLMultiplayerMenu.h"
#include "MultiplayerSessionSubsystem.h"
#include "Components/Button.h"

void UGSLMultiplayerMenu::MenuSetup()
{
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

void UGSLMultiplayerMenu::HostButtonClicked()
{
	UE_LOG(LogTemp , Warning , TEXT("Host ButtonClicked"));
	if (MultiplayerSessionSubsystem)
	{
		MultiplayerSessionSubsystem->CreateSession(4,FString("FreeForAll"));
	}
}

void UGSLMultiplayerMenu::JoinButtonClicked()
{
	UE_LOG(LogTemp , Warning , TEXT("Join ButtonClicked"));
}
