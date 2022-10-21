// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GSLMultiplayerMenu.generated.h"

UCLASS()
class MULTIPLAYERSESSION_API UGSLMultiplayerMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int NumberOfPublicConnection = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")));
	
	
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

	
};
