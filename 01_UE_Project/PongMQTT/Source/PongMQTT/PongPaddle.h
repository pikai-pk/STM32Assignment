#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PongPaddle.generated.h"

class UMaterialInterface;
class UStaticMeshComponent;

UCLASS()
class PONGMQTT_API APongPaddle : public AActor
{
	GENERATED_BODY()

public:
	APongPaddle();

	/* Updates the paddle position every frame using the current move input. */
	virtual void Tick(float DeltaTime) override;

	/* Sets which player this paddle belongs to. Player1 is bottom, Player2 is top. */
	UFUNCTION(BlueprintCallable, Category = "Pong|Paddle")
	void SetPlayerId(int32 InPlayerId);

	/* Returns this paddle's player id. */
	UFUNCTION(BlueprintPure, Category = "Pong|Paddle")
	int32 GetPlayerId() const;

	/* Sets left/right movement input. Expected range is -1..1. */
	UFUNCTION(BlueprintCallable, Category = "Pong|Paddle")
	void SetMoveInput(float InMoveInput);

	/* Moves the paddle by one fixed input step, used by STM32 button press events. */
	UFUNCTION(BlueprintCallable, Category = "Pong|Paddle")
	void NudgeByInput(float InMoveInput);

	/* Returns the current movement input. */
	UFUNCTION(BlueprintPure, Category = "Pong|Paddle")
	float GetMoveInput() const;

	/* Returns the half width used by ball collision checks. */
	UFUNCTION(BlueprintPure, Category = "Pong|Paddle")
	float GetHalfWidth() const;

	/* Configures horizontal movement limits so the paddle cannot cross the side walls. */
	UFUNCTION(BlueprintCallable, Category = "Pong|Paddle")
	void ConfigureMovementBounds(float InMinX, float InMaxX);

	/* Enables or disables debug paddle length from PongField or Blueprint. */
	UFUNCTION(BlueprintCallable, Category = "Pong|Debug")
	void SetDebugPaddleSizeEnabled(bool bEnabled, float InDebugHalfWidth);

	/* Resets the paddle to its configured start location and clears input. */
	UFUNCTION(BlueprintCallable, Category = "Pong|Paddle")
	void ResetPaddle();

	/* Updates the paddle mesh length. Debug mode uses DebugHalfWidth; normal mode restores DefaultHalfWidth. */
	UFUNCTION(BlueprintCallable, Category = "Pong|Debug")
	void ApplyDebugPaddleSettings();

protected:
	/* Caches the starting transform after the actor is placed in the level. */
	virtual void BeginPlay() override;

	/* Applies debug paddle settings when values are changed in the editor Details panel. */
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	/* Applies player-colored material to the paddle mesh. */
	void ApplyVisualMaterial();

	UPROPERTY(VisibleAnywhere, Category = "Pong|Components")
	UStaticMeshComponent* PaddleMesh;

	UPROPERTY(EditAnywhere, Category = "Pong|Paddle")
	int32 PlayerId = 1;

	UPROPERTY(EditAnywhere, Category = "Pong|Paddle")
	float MoveSpeed = 900.0f;

	UPROPERTY(EditAnywhere, Category = "Pong|Paddle")
	float ButtonStepDistance = 80.0f;

	UPROPERTY(EditAnywhere, Category = "Pong|Paddle")
	float MinX = -650.0f;

	UPROPERTY(EditAnywhere, Category = "Pong|Paddle")
	float MaxX = 650.0f;

	UPROPERTY(EditAnywhere, Category = "Pong|Paddle")
	float HalfWidth = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pong|Debug", meta = (AllowPrivateAccess = "true"))
	bool bDebugPaddleSize = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pong|Debug", meta = (AllowPrivateAccess = "true", ClampMin = "40.0", UIMin = "40.0"))
	float DebugHalfWidth = 260.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pong|Debug", meta = (AllowPrivateAccess = "true"))
	float DefaultHalfWidth = 160.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pong|Debug", meta = (AllowPrivateAccess = "true"))
	FVector DefaultMeshScale = FVector(3.2f, 0.28f, 0.18f);

	UPROPERTY(EditDefaultsOnly, Category = "Pong|Materials")
	UMaterialInterface* BaseVisualMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "Pong|Materials")
	FLinearColor Player1Color = FLinearColor(0.08f, 0.82f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, Category = "Pong|Materials")
	FLinearColor Player2Color = FLinearColor(0.42f, 1.0f, 0.32f, 1.0f);

	UPROPERTY(BlueprintReadOnly, Category = "Pong|Paddle", meta = (AllowPrivateAccess = "true"))
	float MoveInput = 0.0f;

	FVector StartLocation = FVector::ZeroVector;
};
