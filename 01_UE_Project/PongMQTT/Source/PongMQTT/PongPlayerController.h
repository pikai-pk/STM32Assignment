#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PongPlayerController.generated.h"

UCLASS()
class PONGMQTT_API APongPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APongPlayerController();

	/* Console command: type Connect to simulate one STM32 board joining the MQTT game. */
	UFUNCTION(Exec)
	void Connect();

	/* Console command: type DebugGaming to toggle local keyboard control simulation. */
	UFUNCTION(Exec)
	void DebugGaming();

protected:
	/* Configures the viewport/input mode when gameplay begins. */
	virtual void BeginPlay() override;

	/* Binds temporary keyboard controls for local testing before MQTT is connected. */
	virtual void SetupInputComponent() override;

	/* Polls debug keyboard keys directly while DebugGaming mode is enabled. */
	virtual void Tick(float DeltaSeconds) override;

private:
	/* Handles Player1 left/right keyboard axis for local testing. */
	void MovePlayer1(float Value);

	/* Handles Player2 left/right keyboard axis for local testing. */
	void MovePlayer2(float Value);

	/* Applies W/S and Up/Down keys without relying on project input mappings. */
	void PollDebugGamingInput();

	bool bDebugGamingInputEnabled = false;
};
