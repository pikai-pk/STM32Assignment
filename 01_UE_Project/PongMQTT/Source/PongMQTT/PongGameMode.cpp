#include "PongGameMode.h"
#include "PongHUD.h"
#include "PongMqttClient.h"
#include "PongPawn.h"
#include "PongPlayerController.h"

APongGameMode::APongGameMode()
{
	DefaultPawnClass = APongPawn::StaticClass();
	PlayerControllerClass = APongPlayerController::StaticClass();
	HUDClass = APongHUD::StaticClass();
}

/* Called when the level starts; initializes the match state for the Pong scene. */
void APongGameMode::StartPlay()
{
	Super::StartPlay();

	PongMatchState = EPongMatchState::Waiting;
	WinnerPlayerId = 0;
	LoserPlayerId = 0;
	if (GetWorld() != nullptr)
	{
		GetWorld()->SpawnActor<APongMqttClient>();
	}
	UE_LOG(LogTemp, Log, TEXT("PongGameMode waiting for MQTT boards."));
}

/* Starts or restarts the Pong match state. */
void APongGameMode::StartMatch()
{
	PongMatchState = EPongMatchState::Running;
	WinnerPlayerId = 0;
	LoserPlayerId = 0;
}

/* Ends the match and records winner/loser ids for UI and MQTT result publishing. */
void APongGameMode::EndMatch(int32 InWinnerPlayerId, int32 InLoserPlayerId)
{
	if (PongMatchState == EPongMatchState::GameOver)
	{
		return;
	}

	PongMatchState = EPongMatchState::GameOver;
	WinnerPlayerId = InWinnerPlayerId;
	LoserPlayerId = InLoserPlayerId;

	UE_LOG(LogTemp, Log, TEXT("Pong match ended. Winner=%d, Loser=%d"), WinnerPlayerId, LoserPlayerId);
}

/* Returns whether paddle input should currently affect the game. */
bool APongGameMode::IsMatchRunning() const
{
	return PongMatchState == EPongMatchState::Running;
}

/* Returns the current high-level match state. */
EPongMatchState APongGameMode::GetPongMatchState() const
{
	return PongMatchState;
}

/* Returns the player id that won the latest completed match. */
int32 APongGameMode::GetWinnerPlayerId() const
{
	return WinnerPlayerId;
}

/* Returns the player id that lost the latest completed match. */
int32 APongGameMode::GetLoserPlayerId() const
{
	return LoserPlayerId;
}
