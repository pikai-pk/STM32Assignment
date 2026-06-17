#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PongField.generated.h"

class APongBall;
class APongPaddle;
class UCameraComponent;
class UChildActorComponent;
class UMaterialInterface;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class PONGMQTT_API APongField : public AActor
{
	GENERATED_BODY()

public:
	APongField();

	/* Applies field dimensions to walls, paddles, ball, and camera when dragged or edited in the level. */
	virtual void OnConstruction(const FTransform& Transform) override;

	/* Sets this field camera as the active editor/play camera when the level starts. */
	virtual void BeginPlay() override;

	/* Returns Player1 paddle spawned by the field. */
	UFUNCTION(BlueprintPure, Category = "Pong|Field")
	APongPaddle* GetPlayer1Paddle() const;

	/* Returns Player2 paddle spawned by the field. */
	UFUNCTION(BlueprintPure, Category = "Pong|Field")
	APongPaddle* GetPlayer2Paddle() const;

	/* Returns the ball spawned by the field. */
	UFUNCTION(BlueprintPure, Category = "Pong|Field")
	APongBall* GetBall() const;

private:
	/* Finds the actor instance spawned by a ChildActorComponent. */
	template <typename T>
	T* GetChildActorAs(const UChildActorComponent* Component) const
	{
		return Component ? Cast<T>(Component->GetChildActor()) : nullptr;
	}

	/* Updates mesh component scale and relative location using field dimensions. */
	void BuildVisualField();

	/* Configures generated child actors with player ids and field bounds. */
	void ConfigureGeneratedActors();

	/* Applies runtime material colors to the field meshes. */
	void ApplyVisualMaterials();

	UPROPERTY(VisibleAnywhere, Category = "Pong|Components")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Pong|Components")
	UStaticMeshComponent* FieldPlane;

	UPROPERTY(VisibleAnywhere, Category = "Pong|Components")
	UStaticMeshComponent* LeftWall;

	UPROPERTY(VisibleAnywhere, Category = "Pong|Components")
	UStaticMeshComponent* RightWall;

	UPROPERTY(VisibleAnywhere, Category = "Pong|Components")
	UStaticMeshComponent* Player1LoseLine;

	UPROPERTY(VisibleAnywhere, Category = "Pong|Components")
	UStaticMeshComponent* Player2LoseLine;

	UPROPERTY(VisibleAnywhere, Category = "Pong|Components")
	UChildActorComponent* Player1PaddleComponent;

	UPROPERTY(VisibleAnywhere, Category = "Pong|Components")
	UChildActorComponent* Player2PaddleComponent;

	UPROPERTY(VisibleAnywhere, Category = "Pong|Components")
	UChildActorComponent* BallComponent;

	UPROPERTY(VisibleAnywhere, Category = "Pong|Components")
	UCameraComponent* FieldCamera;

	UPROPERTY(EditAnywhere, Category = "Pong|Field")
	float HalfWidth = 850.0f;

	UPROPERTY(EditAnywhere, Category = "Pong|Field")
	float HalfHeight = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Pong|Field")
	float PaddleOffsetY = 760.0f;

	UPROPERTY(EditAnywhere, Category = "Pong|Field")
	float WallThickness = 40.0f;

	UPROPERTY(EditAnywhere, Category = "Pong|Field")
	float PaddleHalfWidth = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pong|Debug", meta = (AllowPrivateAccess = "true"))
	bool bDebugPaddleSize = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pong|Debug", meta = (AllowPrivateAccess = "true", ClampMin = "40.0", UIMin = "40.0"))
	float DebugPaddleHalfWidth = 260.0f;

	UPROPERTY(EditAnywhere, Category = "Pong|Camera")
	float CameraHeight = 2200.0f;

	UPROPERTY(EditAnywhere, Category = "Pong|Camera")
	float CameraOrthoWidth = 2300.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Pong|Materials")
	UMaterialInterface* BaseVisualMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "Pong|Materials")
	FLinearColor FieldColor = FLinearColor(0.55f, 0.65f, 0.78f, 1.0f);

	UPROPERTY(EditAnywhere, Category = "Pong|Materials")
	FLinearColor WallColor = FLinearColor(0.06f, 0.09f, 0.14f, 1.0f);

	UPROPERTY(EditAnywhere, Category = "Pong|Materials")
	FLinearColor LoseLineColor = FLinearColor(0.08f, 0.75f, 1.0f, 1.0f);
};
