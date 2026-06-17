#include "PongPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"

APongPawn::APongPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SceneRoot);
	Camera->SetRelativeLocation(CameraLocation);
	Camera->SetRelativeRotation(CameraRotation);
	Camera->ProjectionMode = ECameraProjectionMode::Orthographic;
	Camera->OrthoWidth = 2200.0f;

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

/* Enables ticking once the pawn is in the level. */
void APongPawn::BeginPlay()
{
	Super::BeginPlay();

	if (Camera != nullptr)
	{
		Camera->SetRelativeLocation(CameraLocation);
		Camera->SetRelativeRotation(CameraRotation);
	}
}

/* Updates debug pawn movement each frame; paddle actors can later consume the same input values. */
void APongPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Intentionally empty for now: this pawn owns camera and input state.
	// Paddle movement logic will consume Player1MoveInput/Player2MoveInput in the next game object pass.
}

/* Applies local-test or MQTT move input for one player. PlayerId is 1 or 2, MoveValue is -1..1. */
void APongPawn::SetPlayerMoveInput(int32 PlayerId, float MoveValue)
{
	const float ClampedValue = FMath::Clamp(MoveValue, -1.0f, 1.0f);

	if (PlayerId == 1)
	{
		Player1MoveInput = ClampedValue;
	}
	else if (PlayerId == 2)
	{
		Player2MoveInput = ClampedValue;
	}
}

/* Reads the current move input for one player. */
float APongPawn::GetPlayerMoveInput(int32 PlayerId) const
{
	if (PlayerId == 1)
	{
		return Player1MoveInput;
	}

	if (PlayerId == 2)
	{
		return Player2MoveInput;
	}

	return 0.0f;
}

/* Clears both players' move input, usually used when the match ends. */
void APongPawn::ClearMoveInput()
{
	Player1MoveInput = 0.0f;
	Player2MoveInput = 0.0f;
}
