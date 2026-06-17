#include "PongGameManager.h"
#include "PongBall.h"
#include "PongField.h"
#include "PongGameMode.h"
#include "PongPaddle.h"
#include "PongPawn.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

APongGameManager::APongGameManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

/* Resolves level references and starts the first match. */
void APongGameManager::BeginPlay()
{
	Super::BeginPlay();

	ResolveSceneReferences();
	bWaitingForBoards = true;
	bCountdownRunning = false;
	bGameOver = false;
	ConnectedBoardCount = 0;

	if (Ball != nullptr)
	{
		Ball->SetPaddles(Player1Paddle, Player2Paddle);
		Ball->ResetBall(false);
	}
}

/* Coordinates paddle input, ball loss checks, and match result reporting. */
void APongGameManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bGameOver)
	{
		return;
	}

	SyncInputFromPawn();
	CheckBallResult();
}

/* Starts a new match and resets paddles/ball. */
void APongGameManager::StartNewMatch()
{
	bGameOver = false;
	bWaitingForBoards = false;
	bCountdownRunning = false;
	CountdownSeconds = 0;
	WinnerId = 0;
	LoserId = 0;

	if (Player1Paddle != nullptr)
	{
		Player1Paddle->SetPlayerId(1);
		Player1Paddle->ResetPaddle();
	}

	if (Player2Paddle != nullptr)
	{
		Player2Paddle->SetPlayerId(2);
		Player2Paddle->ResetPaddle();
	}

	if (Ball != nullptr)
	{
		Ball->SetPaddles(Player1Paddle, Player2Paddle);
		Ball->ResetBall(true);
	}

	if (APongGameMode* PongGameMode = GetWorld()->GetAuthGameMode<APongGameMode>())
	{
		PongGameMode->StartMatch();
	}

	OnMatchStarted.Broadcast();
}

/* Called by the MQTT layer when a STM32 STATUS message is received. */
void APongGameManager::HandlePlayerStatusFromMqtt(int32 PlayerId, const FString& IpAddress)
{
	SetPlayerIp(PlayerId, IpAddress);

	const int32 PreviousCount = ConnectedBoardCount;

	if (PlayerId == 1)
	{
		bPlayer1Joined = true;
	}
	else if (PlayerId == 2)
	{
		bPlayer2Joined = true;
	}

	ConnectedBoardCount = (bPlayer1Joined ? 1 : 0) + (bPlayer2Joined ? 1 : 0);
	if (ConnectedBoardCount != PreviousCount)
	{
		OnBoardCountChanged.Broadcast(ConnectedBoardCount);
	}

	TryStartCountdown();
}

/* Called by the MQTT layer when a STM32 INPUT message is received. */
void APongGameManager::HandlePlayerInputFromMqtt(int32 PlayerId, int32 MoveValue)
{
	if (bWaitingForBoards || bCountdownRunning || bGameOver)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Pong MQTT input ignored while not running. Player=%d Move=%d"), PlayerId, MoveValue);
		return;
	}

	const int32 ClampedMove = FMath::Clamp(MoveValue, -1, 1);
	if (ClampedMove == 0)
	{
		SetPlayerMoveInput(PlayerId, 0.0f);
		return;
	}

	if (Player1Paddle == nullptr || Player2Paddle == nullptr || PongPawn == nullptr)
	{
		ResolveSceneReferences();
	}

	if (PlayerId == 1 && Player1Paddle != nullptr)
	{
		Player1Paddle->NudgeByInput(static_cast<float>(ClampedMove));
	}
	else if (PlayerId == 2 && Player2Paddle != nullptr)
	{
		Player2Paddle->NudgeByInput(static_cast<float>(ClampedMove));
	}

	if (PongPawn != nullptr)
	{
		PongPawn->SetPlayerMoveInput(PlayerId, 0.0f);
	}

	UE_LOG(LogTemp, Log, TEXT("Pong MQTT button step applied. Player=%d Move=%d"), PlayerId, ClampedMove);
}

/* Applies MQTT or local input to one player's paddle. */
void APongGameManager::SetPlayerMoveInput(int32 PlayerId, float MoveValue)
{
	if (Player1Paddle == nullptr || Player2Paddle == nullptr || PongPawn == nullptr)
	{
		ResolveSceneReferences();
	}

	if (PlayerId == 1 && Player1Paddle != nullptr)
	{
		Player1Paddle->SetMoveInput(MoveValue);
	}
	else if (PlayerId == 2 && Player2Paddle != nullptr)
	{
		Player2Paddle->SetMoveInput(MoveValue);
	}

	if (PongPawn != nullptr)
	{
		PongPawn->SetPlayerMoveInput(PlayerId, MoveValue);
	}
}

/* Saves the latest IP reported by a player's STM32 STATUS message. */
void APongGameManager::SetPlayerIp(int32 PlayerId, const FString& IpAddress)
{
	if (PlayerId == 1)
	{
		Player1Ip = IpAddress;
	}
	else if (PlayerId == 2)
	{
		Player2Ip = IpAddress;
	}
}

/* Ends the match, records result variables, and notifies UI/MQTT layers. */
void APongGameManager::EndMatch(int32 WinnerPlayerId, int32 LoserPlayerId)
{
	if (bGameOver)
	{
		return;
	}

	bGameOver = true;
	WinnerId = WinnerPlayerId;
	LoserId = LoserPlayerId;

	if (Player1Paddle != nullptr)
	{
		Player1Paddle->SetMoveInput(0.0f);
	}

	if (Player2Paddle != nullptr)
	{
		Player2Paddle->SetMoveInput(0.0f);
	}

	if (Ball != nullptr)
	{
		Ball->StopBall();
	}

	if (APongGameMode* PongGameMode = GetWorld()->GetAuthGameMode<APongGameMode>())
	{
		PongGameMode->EndMatch(WinnerId, LoserId);
	}

	OnMatchEnded.Broadcast(WinnerId, LoserId, ResolveIpForPlayer(WinnerId), ResolveIpForPlayer(LoserId));
	UE_LOG(LogTemp, Log, TEXT("PongGameManager result. Winner=%d, Loser=%d"), WinnerId, LoserId);

	StartNextRoundCountdownIfReady();
}

/* Returns true after a match has ended. */
bool APongGameManager::IsGameOver() const
{
	return bGameOver;
}

/* Returns true while waiting for both STM32 boards to join. */
bool APongGameManager::IsWaitingForBoards() const
{
	return bWaitingForBoards;
}

/* Returns true while the 5-second countdown is running. */
bool APongGameManager::IsCountdownRunning() const
{
	return bCountdownRunning;
}

/* Returns how many STM32 boards have joined the game. */
int32 APongGameManager::GetConnectedBoardCount() const
{
	return ConnectedBoardCount;
}

/* Returns current countdown value. 0 means no countdown is active. */
int32 APongGameManager::GetCountdownSeconds() const
{
	return CountdownSeconds;
}

/* Returns the winner id for UI display. */
int32 APongGameManager::GetWinnerPlayerId() const
{
	return WinnerId;
}

/* Returns the loser id for UI display. */
int32 APongGameManager::GetLoserPlayerId() const
{
	return LoserId;
}

/* Returns the stored IP address for the requested player id. */
FString APongGameManager::GetPlayerIp(int32 PlayerId) const
{
	return ResolveIpForPlayer(PlayerId);
}

/* Finds paddles and ball in the level if references were not assigned manually. */
void APongGameManager::ResolveSceneReferences()
{
	if (APongField* Field = Cast<APongField>(UGameplayStatics::GetActorOfClass(this, APongField::StaticClass())))
	{
		if (Player1Paddle == nullptr)
		{
			Player1Paddle = Field->GetPlayer1Paddle();
		}

		if (Player2Paddle == nullptr)
		{
			Player2Paddle = Field->GetPlayer2Paddle();
		}

		if (Ball == nullptr)
		{
			Ball = Field->GetBall();
		}
	}

	for (TActorIterator<APongPaddle> It(GetWorld()); It; ++It)
	{
		APongPaddle* Paddle = *It;
		if (Paddle == nullptr)
		{
			continue;
		}

		if (Paddle->GetPlayerId() == 1 && Player1Paddle == nullptr)
		{
			Player1Paddle = Paddle;
		}
		else if (Paddle->GetPlayerId() == 2 && Player2Paddle == nullptr)
		{
			Player2Paddle = Paddle;
		}
	}

	if (Ball == nullptr)
	{
		Ball = Cast<APongBall>(UGameplayStatics::GetActorOfClass(this, APongBall::StaticClass()));
	}

	PongPawn = Cast<APongPawn>(UGameplayStatics::GetPlayerPawn(this, 0));
}

/* Copies current PongPawn input values to both paddles. */
void APongGameManager::SyncInputFromPawn()
{
	if (bWaitingForBoards || bCountdownRunning || bGameOver)
	{
		if (Player1Paddle != nullptr)
		{
			Player1Paddle->SetMoveInput(0.0f);
		}

		if (Player2Paddle != nullptr)
		{
			Player2Paddle->SetMoveInput(0.0f);
		}

		return;
	}

	if (PongPawn == nullptr)
	{
		ResolveSceneReferences();
	}

	if (PongPawn == nullptr)
	{
		return;
	}

	if (Player1Paddle != nullptr)
	{
		Player1Paddle->SetMoveInput(PongPawn->GetPlayerMoveInput(1));
	}

	if (Player2Paddle != nullptr)
	{
		Player2Paddle->SetMoveInput(PongPawn->GetPlayerMoveInput(2));
	}
}

/* Checks ball lose-line state and ends the match when needed. */
void APongGameManager::CheckBallResult()
{
	if (Ball == nullptr)
	{
		return;
	}

	const int32 LosingPlayerId = Ball->GetLosingPlayerId();
	if (LosingPlayerId == 1)
	{
		UE_LOG(LogTemp, Log, TEXT("Pong ball entered Player1 edge. Player2 scores."));
		EndMatch(2, 1);
	}
	else if (LosingPlayerId == 2)
	{
		UE_LOG(LogTemp, Log, TEXT("Pong ball entered Player2 edge. Player1 scores."));
		EndMatch(1, 2);
	}
}

/* Returns the IP string for winner/loser result output. */
FString APongGameManager::ResolveIpForPlayer(int32 PlayerId) const
{
	if (PlayerId == 1)
	{
		return Player1Ip;
	}

	if (PlayerId == 2)
	{
		return Player2Ip;
	}

	return TEXT("Unknown");
}

/* Starts the countdown once both STM32 boards are online. */
void APongGameManager::TryStartCountdown()
{
	if (!bWaitingForBoards || bCountdownRunning || bGameOver)
	{
		return;
	}

	if (!bPlayer1Joined || !bPlayer2Joined)
	{
		return;
	}

	bCountdownRunning = true;
	CountdownSeconds = 5;
	OnCountdownChanged.Broadcast(CountdownSeconds);
	GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &APongGameManager::AdvanceCountdown, 1.0f, true);
}

/* Runs one countdown tick and starts the match at zero. */
void APongGameManager::AdvanceCountdown()
{
	CountdownSeconds = FMath::Max(CountdownSeconds - 1, 0);
	OnCountdownChanged.Broadcast(CountdownSeconds);

	if (CountdownSeconds <= 0)
	{
		GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
		StartNewMatch();
	}
}

/* Starts a 5-second next-round countdown after game over while both boards remain online. */
void APongGameManager::StartNextRoundCountdownIfReady()
{
	if (!bPlayer1Joined || !bPlayer2Joined)
	{
		return;
	}

	bWaitingForBoards = false;
	bCountdownRunning = true;
	CountdownSeconds = 5;
	OnCountdownChanged.Broadcast(CountdownSeconds);
	GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
	GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &APongGameManager::AdvanceCountdown, 1.0f, true);
	UE_LOG(LogTemp, Log, TEXT("Both boards still online. Next round countdown started."));
}
