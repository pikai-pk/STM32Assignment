#include "PongPlayerController.h"
#include "PongGameManager.h"
#include "PongPawn.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"

APongPlayerController::APongPlayerController()
{
	bShowMouseCursor = false;
	PrimaryActorTick.bCanEverTick = true;
}

/* Console command: type Connect to simulate one STM32 board joining the MQTT game. */
void APongPlayerController::Connect()
{
	APongGameManager* GameManager = Cast<APongGameManager>(UGameplayStatics::GetActorOfClass(this, APongGameManager::StaticClass()));
	if (GameManager == nullptr)
	{
		GameManager = GetWorld()->SpawnActor<APongGameManager>(APongGameManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (GameManager == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Connect command failed: PongGameManager was not found and could not be spawned."));
			return;
		}

		UE_LOG(LogTemp, Log, TEXT("Connect command spawned PongGameManager automatically."));
	}

	const int32 ConnectedCount = GameManager->GetConnectedBoardCount();
	if (ConnectedCount >= 2)
	{
		UE_LOG(LogTemp, Log, TEXT("Connect command ignored: both simulated STM32 boards are already connected."));
		return;
	}

	const int32 PlayerId = ConnectedCount + 1;
	const FString SimulatedIp = PlayerId == 1 ? TEXT("192.168.1.10") : TEXT("192.168.1.11");
	GameManager->HandlePlayerStatusFromMqtt(PlayerId, SimulatedIp);

	UE_LOG(LogTemp, Log, TEXT("Connect command simulated STM32 board. Player=%d IP=%s"), PlayerId, *SimulatedIp);
}

/* Console command: type DebugGaming to toggle local keyboard control simulation. */
void APongPlayerController::DebugGaming()
{
	bDebugGamingInputEnabled = true;

	if (APongGameManager* GameManager = Cast<APongGameManager>(UGameplayStatics::GetActorOfClass(this, APongGameManager::StaticClass())))
	{
		if (GameManager->IsGameOver() && GameManager->GetConnectedBoardCount() >= 2)
		{
			GameManager->StartNewMatch();
			UE_LOG(LogTemp, Log, TEXT("DebugGaming restarted the match for keyboard testing."));
		}
	}

	UE_LOG(LogTemp, Log, TEXT("DebugGaming keyboard simulation is now %s. Player1=W/S Player2=Up/Down."),
		bDebugGamingInputEnabled ? TEXT("ON") : TEXT("OFF"));
}

/* Configures the viewport/input mode when gameplay begins. */
void APongPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

}

/* Binds temporary keyboard controls for local testing before MQTT is connected. */
void APongPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent == nullptr)
	{
		return;
	}

	InputComponent->BindAxis(TEXT("Player1Move"), this, &APongPlayerController::MovePlayer1);
	InputComponent->BindAxis(TEXT("Player2Move"), this, &APongPlayerController::MovePlayer2);
}

/* Polls debug keyboard keys directly while DebugGaming mode is enabled. */
void APongPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	PollDebugGamingInput();
}

/* Handles Player1 left/right keyboard axis for local testing. */
void APongPlayerController::MovePlayer1(float Value)
{
	if (!bDebugGamingInputEnabled)
	{
		return;
	}

	if (APongPawn* PongPawn = Cast<APongPawn>(GetPawn()))
	{
		PongPawn->SetPlayerMoveInput(1, Value);
	}
}

/* Handles Player2 left/right keyboard axis for local testing. */
void APongPlayerController::MovePlayer2(float Value)
{
	if (!bDebugGamingInputEnabled)
	{
		return;
	}

	if (APongPawn* PongPawn = Cast<APongPawn>(GetPawn()))
	{
		PongPawn->SetPlayerMoveInput(2, Value);
	}
}

/* Applies W/S and Up/Down keys without relying on project input mappings. */
void APongPlayerController::PollDebugGamingInput()
{
	if (!bDebugGamingInputEnabled)
	{
		return;
	}

	APongPawn* PongPawn = Cast<APongPawn>(GetPawn());
	if (PongPawn == nullptr)
	{
		return;
	}

	float Player1Value = 0.0f;
	if (IsInputKeyDown(EKeys::W))
	{
		Player1Value += 1.0f;
	}
	if (IsInputKeyDown(EKeys::S))
	{
		Player1Value -= 1.0f;
	}

	float Player2Value = 0.0f;
	if (IsInputKeyDown(EKeys::Up))
	{
		Player2Value += 1.0f;
	}
	if (IsInputKeyDown(EKeys::Down))
	{
		Player2Value -= 1.0f;
	}

	PongPawn->SetPlayerMoveInput(1, FMath::Clamp(Player1Value, -1.0f, 1.0f));
	PongPawn->SetPlayerMoveInput(2, FMath::Clamp(Player2Value, -1.0f, 1.0f));
}
