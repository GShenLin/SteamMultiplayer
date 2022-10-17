// Copyright Epic Games, Inc. All Rights Reserved.

#include "SteamTestCharacter.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
//#include "Interfaces/OnlineSessionInterface.h"

//////////////////////////////////////////////////////////////////////////
// ASteamTestCharacter

ASteamTestCharacter::ASteamTestCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	CreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this , &ThisClass::OnCreateSessionComplete);
	FindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this , &ThisClass::OnFindSessionComplete);
	JoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this , &ThisClass::OnJoinSessionComplete);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASteamTestCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &ASteamTestCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &ASteamTestCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &ASteamTestCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &ASteamTestCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ASteamTestCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ASteamTestCharacter::TouchStopped);
}

void ASteamTestCharacter::BeginPlay()
{

	Super::BeginPlay();
	
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		OnlineSessionInterface =  OnlineSubsystem->GetSessionInterface();
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,
				15.f,
				FColor::Blue,
				FString::Printf(TEXT("Found Subsystem %s"), *OnlineSubsystem->GetSubsystemName().ToString())
				);
		}
	}

}

void ASteamTestCharacter::CreateGameSession()
{
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}
	FNamedOnlineSession * ExistingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		OnlineSessionInterface->DestroySession(NAME_GameSession);
	}

	OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
	
	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSettings->bIsLANMatch = false;  //是否局域网 如果不设置这个 无法成功创建Session
	SessionSettings->NumPublicConnections = 4;  //最多几人连接
	SessionSettings->bAllowJoinInProgress = true; // 可以链接
	SessionSettings->bAllowJoinViaPresence = true; // 通过Steam的区域加入房间？
	SessionSettings->bShouldAdvertise  = true;   // 发起广告通知其他玩家
	SessionSettings->bUsesPresence = true; //使用Steam的区域划分
	SessionSettings->bUseLobbiesIfAvailable = true; // 很关键 不设置这个无法创建成功
	//查找特定类型的比赛 具体多看下Set函数 支持多种值得类型参数 这里只是一个示例 最后一个参数表示同时可以通过OnlineServeice和ping找到Session
	SessionSettings->Set(FName("MatchType")  , FString("FreeForAll"),EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	
	
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	OnlineSessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(),NAME_GameSession, *SessionSettings);
}

void ASteamTestCharacter::JoinGameSession()
{
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}

	//将代理绑定到接口中
	OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = 10000; //如果使用的SteamAppID是480 是SpaceWar的测试用ID 会搜索到很多结果
	SessionSearch->bIsLanQuery = false; //局域网？

	//过滤搜索结果 只搜索特定状态下的Session
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE,true,EOnlineComparisonOp::Equals);
	
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	OnlineSessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(),SessionSearch.ToSharedRef());
}

void ASteamTestCharacter::OnCreateSessionComplete(FName SessionName, bool Success)
{
	if (Success)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
-1,15.f,FColor::Red,
FString::Printf(TEXT("Create Session Success : %s !") , *SessionName.ToString())
			);
		}
		UWorld * World  = GetWorld();
		if (World)
		{
			bool bSuccessfulTravel  = World->ServerTravel(FString("/Game/Level/Lobby?listen"));
			if (bSuccessfulTravel)
			{
				UE_LOG(LogTemp , Warning , TEXT("SuccessfulTravel"));
			}
			else
			{
				UE_LOG(LogTemp , Warning , TEXT("Travel Failure"));
			}
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
-1,15.f,FColor::Red,
FString(TEXT("Fail on create Session !"))
			);
		}
	}
}

void ASteamTestCharacter::OnFindSessionComplete(bool bWasSuccessful)
{
	
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}
	
	
	for (FOnlineSessionSearchResult Result : SessionSearch->SearchResults)
	{
		FString Id = Result.GetSessionIdStr();
		FString User = Result.Session.OwningUserName;

		FString MatchType;
		Result.Session.SessionSettings.Get(FName("MatchType")  , MatchType);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15,
				FColor::Blue,
				FString::Printf(TEXT("ID: %s , UserName: %s") , *Id , *User )
				
			);
		}
		if (MatchType == FString("FreeForAll"))
		{
			// 这里就已经找到房间
			//在查找成功后 绑定加入的代理
			OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

			//加入房间
			const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
			OnlineSessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(),NAME_GameSession,Result);
		}
	}
}

void ASteamTestCharacter::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type ResultType)
{
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}
	FString Address;
	bool bJoinSuccessful = OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession,Address);
	if (bJoinSuccessful)
	{
		UE_LOG(LogTemp , Warning , TEXT("JoinSuccessful : %s"), *Address);
		APlayerController * PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
		if (PlayerController)
		{
			PlayerController->ClientTravel(Address,ETravelType::TRAVEL_Absolute);
		}
	}
}

void ASteamTestCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void ASteamTestCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void ASteamTestCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ASteamTestCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ASteamTestCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ASteamTestCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
