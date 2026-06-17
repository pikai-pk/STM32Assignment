#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PongGameManager.generated.h"

class APongBall;
class APongPaddle;
class APongPawn;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FPongMatchEndedSignature, int32, WinnerPlayerId, int32, LoserPlayerId, const FString&, WinnerIp, const FString&, LoserIp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPongBoardCountChangedSignature, int32, ConnectedBoardCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPongCountdownChangedSignature, int32, CountdownSeconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPongMatchStartedSignature);

UCLASS()
class PONGMQTT_API APongGameManager : public AActor
{
	GENERATED_BODY()

public:
	APongGameManager();

	/* Coordinates paddle input, ball loss checks, and match result reporting. */
	virtual void Tick(float DeltaTime) override;

	/* Starts a new match and resets paddles/ball. */
	UFUNCTION(BlueprintCallable, Category = "Pong|Game")
	void StartNewMatch();

	/* Called by the MQTT layer when a STM32 STATUS message is received. */
	UFUNCTION(BlueprintCallable, Category = "Pong|MQTT")
	void HandlePlayerStatusFromMqtt(int32 PlayerId, const FString& IpAddress);

	/* Called by the MQTT layer when a STM32 INPUT message is received. */
	UFUNCTION(BlueprintCallable, Category = "Pong|MQTT")
	void HandlePlayerInputFromMqtt(int32 PlayerId, int32 MoveValue);

	/* Applies MQTT or local input to one player's paddle. */
	UFUNCTION(BlueprintCallable, Category = "Pong|Input")
	void SetPlayerMoveInput(int32 PlayerId, float MoveValue);

	/* Saves the latest IP reported by a player's STM32 STATUS message. */
	UFUNCTION(BlueprintCallable, Category = "Pong|MQTT")
	void SetPlayerIp(int32 PlayerId, const FString& IpAddress);

	/* Ends the match, records result variables, and notifies UI/MQTT layers. */
	UFUNCTION(BlueprintCallable, Category = "Pong|Game")
	void EndMatch(int32 WinnerPlayerId, int32 LoserPlayerId);

	/* Returns true after a match has ended. */
	UFUNCTION(BlueprintPure, Category = "Pong|Game")
	bool IsGameOver() const;

	/* Returns true while waiting for both STM32 boards to join. */
	UFUNCTION(BlueprintPure, Category = "Pong|UI")
	bool IsWaitingForBoards() const;

	/* Returns true while the 5-second countdown is running. */
	UFUNCTION(BlueprintPure, Category = "Pong|UI")
	bool IsCountdownRunning() const;

	/* Returns how many STM32 boards have joined the game. */
	UFUNCTION(BlueprintPure, Category = "Pong|UI")
	int32 GetConnectedBoardCount() const;

	/* Returns current countdown value. 0 means no countdown is active. */
	UFUNCTION(BlueprintPure, Category = "Pong|UI")
	int32 GetCountdownSeconds() const;

	/* Returns the winner id for UI display. */
	UFUNCTION(BlueprintPure, Category = "Pong|UI")
	int32 GetWinnerPlayerId() const;

	/* Returns the loser id for UI display. */
	UFUNCTION(BlueprintPure, Category = "Pong|UI")
	int32 GetLoserPlayerId() const;

	/* Returns the stored IP address for the requested player id. */
	UFUNCTION(BlueprintPure, Category = "Pong|UI")
	FString GetPlayerIp(int32 PlayerId) const;

	/* Broadcast when the match ends; Widget can bind to this for result UI. */
	UPROPERTY(BlueprintAssignable, Category = "Pong|UI")
	FPongMatchEndedSignature OnMatchEnded;

	/* Broadcast when a STM32 board joins or reconnects. */
	UPROPERTY(BlueprintAssignable, Category = "Pong|UI")
	FPongBoardCountChangedSignature OnBoardCountChanged;

	/* Broadcast every second during the pre-match countdown. */
	UPROPERTY(BlueprintAssignable, Category = "Pong|UI")
	FPongCountdownChangedSignature OnCountdownChanged;

	/* Broadcast when countdown finishes and the ball starts moving. */
	UPROPERTY(BlueprintAssignable, Category = "Pong|UI")
	FPongMatchStartedSignature OnMatchStarted;

protected:
	/* Resolves level references and starts the first match. */
	virtual void BeginPlay() override;

private:
	/* Finds paddles and ball in the level if references were not assigned manually. */
	void ResolveSceneReferences();

	/* Copies current PongPawn input values to both paddles. */
	void SyncInputFromPawn();

	/* Checks ball lose-line state and ends the match when needed. */
	void CheckBallResult();

	/* Returns the IP string for winner/loser result output. */
	FString ResolveIpForPlayer(int32 PlayerId) const;

	/* Starts the countdown once both STM32 boards are online. */
	void TryStartCountdown();

	/* Runs one countdown tick and starts the match at zero. */
	void AdvanceCountdown();

	/* Starts a 5-second next-round countdown after game over while both boards remain online. */
	void StartNextRoundCountdownIfReady();

	UPROPERTY(EditInstanceOnly, Category = "Pong|Scene")
	APongPaddle* Player1Paddle = nullptr;

	UPROPERTY(EditInstanceOnly, Category = "Pong|Scene")
	APongPaddle* Player2Paddle = nullptr;

	UPROPERTY(EditInstanceOnly, Category = "Pong|Scene")
	APongBall* Ball = nullptr;

	UPROPERTY()
	APongPawn* PongPawn = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Pong|UI", meta = (AllowPrivateAccess = "true"))
	int32 WinnerId = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Pong|UI", meta = (AllowPrivateAccess = "true"))
	int32 LoserId = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Pong|UI", meta = (AllowPrivateAccess = "true"))
	FString Player1Ip = TEXT("Unknown");

	UPROPERTY(BlueprintReadOnly, Category = "Pong|UI", meta = (AllowPrivateAccess = "true"))
	FString Player2Ip = TEXT("Unknown");

	UPROPERTY(BlueprintReadOnly, Category = "Pong|UI", meta = (AllowPrivateAccess = "true"))
	bool bPlayer1Joined = false;

	UPROPERTY(BlueprintReadOnly, Category = "Pong|UI", meta = (AllowPrivateAccess = "true"))
	bool bPlayer2Joined = false;

	UPROPERTY(BlueprintReadOnly, Category = "Pong|UI", meta = (AllowPrivateAccess = "true"))
	int32 ConnectedBoardCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Pong|UI", meta = (AllowPrivateAccess = "true"))
	bool bWaitingForBoards = true;

	UPROPERTY(BlueprintReadOnly, Category = "Pong|UI", meta = (AllowPrivateAccess = "true"))
	bool bCountdownRunning = false;

	UPROPERTY(BlueprintReadOnly, Category = "Pong|UI", meta = (AllowPrivateAccess = "true"))
	int32 CountdownSeconds = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Pong|Game", meta = (AllowPrivateAccess = "true"))
	bool bGameOver = false;

	FTimerHandle CountdownTimerHandle;
};
