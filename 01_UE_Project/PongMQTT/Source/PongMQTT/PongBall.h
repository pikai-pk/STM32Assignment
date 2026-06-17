#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PongBall.generated.h"

class APongPaddle;
class UMaterialInterface;
class UStaticMeshComponent;

UCLASS()
class PONGMQTT_API APongBall : public AActor
{
	GENERATED_BODY()

public:
	APongBall();

	/* Moves the ball and checks wall/paddle bounce while active. */
	virtual void Tick(float DeltaTime) override;

	/* Assigns the two paddle actors used for bounce checks. */
	UFUNCTION(BlueprintCallable, Category = "Pong|Ball")
	void SetPaddles(APongPaddle* InPlayer1Paddle, APongPaddle* InPlayer2Paddle);

	/* Starts ball movement from its current location. */
	UFUNCTION(BlueprintCallable, Category = "Pong|Ball")
	void StartBall();

	/* Stops ball movement without changing its location. */
	UFUNCTION(BlueprintCallable, Category = "Pong|Ball")
	void StopBall();

	/* Resets the ball to its start location, optionally starting movement immediately. */
	UFUNCTION(BlueprintCallable, Category = "Pong|Ball")
	void ResetBall(bool bStartImmediately = true);

	/* Returns true when the ball is currently moving. */
	UFUNCTION(BlueprintPure, Category = "Pong|Ball")
	bool IsBallActive() const;

	/* Returns the player id that lost because the ball crossed a lose line, or 0 if no one lost. */
	UFUNCTION(BlueprintPure, Category = "Pong|Ball")
	int32 GetLosingPlayerId() const;

	/* Configures wall bounce limits and lose-line limits from the field actor. */
	UFUNCTION(BlueprintCallable, Category = "Pong|Ball")
	void ConfigureFieldBounds(float InLeftWallX, float InRightWallX, float InPlayer1LoseY, float InPlayer2LoseY);

protected:
	/* Caches start location and keeps the ball paused until the match starts. */
	virtual void BeginPlay() override;

private:
	/* Reflects the ball from left/right walls. */
	void CheckWallBounce();

	/* Reflects the ball from both player paddles. */
	void CheckPaddleBounce();

	/* Reflects the ball from one paddle if the overlap test succeeds. */
	void CheckOnePaddle(APongPaddle* Paddle, int32 DirectionY);

	/* Applies bright material to the ball mesh. */
	void ApplyVisualMaterial();

	UPROPERTY(VisibleAnywhere, Category = "Pong|Components")
	UStaticMeshComponent* BallMesh;

	UPROPERTY(EditAnywhere, Category = "Pong|Ball")
	float Speed = 720.0f;

	UPROPERTY(EditAnywhere, Category = "Pong|Ball")
	float LeftWallX = -800.0f;

	UPROPERTY(EditAnywhere, Category = "Pong|Ball")
	float RightWallX = 800.0f;

	UPROPERTY(EditAnywhere, Category = "Pong|Ball")
	float Player1LoseY = -950.0f;

	UPROPERTY(EditAnywhere, Category = "Pong|Ball")
	float Player2LoseY = 950.0f;

	UPROPERTY(EditAnywhere, Category = "Pong|Ball")
	float PaddleHitHalfHeight = 24.0f;

	UPROPERTY(EditAnywhere, Category = "Pong|Ball")
	float PaddleCollisionShrink = 18.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Pong|Materials")
	UMaterialInterface* BaseVisualMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "Pong|Materials")
	FLinearColor BallColor = FLinearColor(0.95f, 0.98f, 1.0f, 1.0f);

	UPROPERTY(BlueprintReadOnly, Category = "Pong|Ball", meta = (AllowPrivateAccess = "true"))
	FVector2D Velocity = FVector2D(0.45f, 1.0f);

	UPROPERTY(BlueprintReadOnly, Category = "Pong|Ball", meta = (AllowPrivateAccess = "true"))
	bool bActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Pong|Ball", meta = (AllowPrivateAccess = "true"))
	int32 LosingPlayerId = 0;

	UPROPERTY()
	APongPaddle* Player1Paddle = nullptr;

	UPROPERTY()
	APongPaddle* Player2Paddle = nullptr;

	FVector StartLocation = FVector::ZeroVector;
	FVector PreviousLocation = FVector::ZeroVector;
};
