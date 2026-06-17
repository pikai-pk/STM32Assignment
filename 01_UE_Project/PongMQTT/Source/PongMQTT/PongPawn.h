#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PongPawn.generated.h"

class UCameraComponent;
class USceneComponent;

UCLASS()
class PONGMQTT_API APongPawn : public APawn
{
	GENERATED_BODY()

public:
	APongPawn();

	/* Applies local-test or MQTT move input for one player. PlayerId is 1 or 2, MoveValue is -1..1. */
	UFUNCTION(BlueprintCallable, Category = "Pong|Input")
	void SetPlayerMoveInput(int32 PlayerId, float MoveValue);

	/* Reads the current move input for one player. */
	UFUNCTION(BlueprintPure, Category = "Pong|Input")
	float GetPlayerMoveInput(int32 PlayerId) const;

	/* Clears both players' move input, usually used when the match ends. */
	UFUNCTION(BlueprintCallable, Category = "Pong|Input")
	void ClearMoveInput();

protected:
	/* Updates debug pawn movement each frame; paddle actors can later consume the same input values. */
	virtual void Tick(float DeltaTime) override;

	/* Enables ticking once the pawn is in the level. */
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Pong|Components")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Pong|Components")
	UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, Category = "Pong|Camera")
	FVector CameraLocation = FVector(0.0f, 0.0f, 1800.0f);

	UPROPERTY(EditAnywhere, Category = "Pong|Camera")
	FRotator CameraRotation = FRotator(-90.0f, 0.0f, 0.0f);

	UPROPERTY(BlueprintReadOnly, Category = "Pong|Input", meta = (AllowPrivateAccess = "true"))
	float Player1MoveInput = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Pong|Input", meta = (AllowPrivateAccess = "true"))
	float Player2MoveInput = 0.0f;
};
