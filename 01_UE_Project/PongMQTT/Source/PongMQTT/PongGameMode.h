#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PongGameMode.generated.h"

class APongPawn;

UENUM(BlueprintType)
enum class EPongMatchState : uint8
{
	Waiting,
	Running,
	GameOver
};

UCLASS()
class PONGMQTT_API APongGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	APongGameMode();

	/* Called when the level starts; initializes the match state for the Pong scene. */
	virtual void StartPlay() override;

	/* Starts or restarts the Pong match state. */
	UFUNCTION(BlueprintCallable, Category = "Pong|Game")
	void StartMatch();

	/* Ends the match and records winner/loser ids for UI and MQTT result publishing. */
	UFUNCTION(BlueprintCallable, Category = "Pong|Game")
	void EndMatch(int32 InWinnerPlayerId, int32 InLoserPlayerId);

	/* Returns whether paddle input should currently affect the game. */
	UFUNCTION(BlueprintPure, Category = "Pong|Game")
	bool IsMatchRunning() const;

	/* Returns the current high-level match state. */
	UFUNCTION(BlueprintPure, Category = "Pong|Game")
	EPongMatchState GetPongMatchState() const;

	/* Returns the player id that won the latest completed match. */
	UFUNCTION(BlueprintPure, Category = "Pong|Game")
	int32 GetWinnerPlayerId() const;

	/* Returns the player id that lost the latest completed match. */
	UFUNCTION(BlueprintPure, Category = "Pong|Game")
	int32 GetLoserPlayerId() const;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Pong|Game")
	EPongMatchState PongMatchState = EPongMatchState::Waiting;

	UPROPERTY(BlueprintReadOnly, Category = "Pong|Game")
	int32 WinnerPlayerId = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Pong|Game")
	int32 LoserPlayerId = 0;
};
