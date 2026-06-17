#include "PongBall.h"
#include "PongPaddle.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

APongBall::APongBall()
{
	PrimaryActorTick.bCanEverTick = true;

	BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
	SetRootComponent(BallMesh);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialAsset(TEXT("/Game/Material/BasicShapeMaterial.BasicShapeMaterial"));
	if (SphereMesh.Succeeded())
	{
		BallMesh->SetStaticMesh(SphereMesh.Object);
	}
	if (MaterialAsset.Succeeded())
	{
		BaseVisualMaterial = MaterialAsset.Object;
	}

	BallMesh->SetWorldScale3D(FVector(0.28f, 0.28f, 0.28f));
	BallMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BallMesh->SetCastShadow(false);
}

/* Caches start location and keeps the ball paused until the match starts. */
void APongBall::BeginPlay()
{
	Super::BeginPlay();

	ApplyVisualMaterial();
	StartLocation = GetActorLocation();
	ResetBall(false);
}

/* Moves the ball and checks wall/paddle bounce while active. */
void APongBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bActive)
	{
		return;
	}

	FVector Location = GetActorLocation();
	PreviousLocation = Location;
	Location.X += Velocity.X * Speed * DeltaTime;
	Location.Y += Velocity.Y * Speed * DeltaTime;
	SetActorLocation(Location);

	CheckWallBounce();
	CheckPaddleBounce();

	const float BallY = GetActorLocation().Y;
	if (BallY <= Player1LoseY)
	{
		LosingPlayerId = 1;
		StopBall();
	}
	else if (BallY >= Player2LoseY)
	{
		LosingPlayerId = 2;
		StopBall();
	}
}

/* Assigns the two paddle actors used for bounce checks. */
void APongBall::SetPaddles(APongPaddle* InPlayer1Paddle, APongPaddle* InPlayer2Paddle)
{
	Player1Paddle = InPlayer1Paddle;
	Player2Paddle = InPlayer2Paddle;
}

/* Starts ball movement from its current location. */
void APongBall::StartBall()
{
	bActive = true;
	LosingPlayerId = 0;
}

/* Stops ball movement without changing its location. */
void APongBall::StopBall()
{
	bActive = false;
}

/* Resets the ball to its start location, optionally starting movement immediately. */
void APongBall::ResetBall(bool bStartImmediately)
{
	SetActorLocation(StartLocation);
	PreviousLocation = StartLocation;
	Velocity = FVector2D(0.45f, FMath::RandBool() ? 1.0f : -1.0f).GetSafeNormal();
	LosingPlayerId = 0;

	if (bStartImmediately)
	{
		StartBall();
	}
	else
	{
		StopBall();
	}
}

/* Returns true when the ball is currently moving. */
bool APongBall::IsBallActive() const
{
	return bActive;
}

/* Returns the player id that lost because the ball crossed a lose line, or 0 if no one lost. */
int32 APongBall::GetLosingPlayerId() const
{
	return LosingPlayerId;
}

/* Configures wall bounce limits and lose-line limits from the field actor. */
void APongBall::ConfigureFieldBounds(float InLeftWallX, float InRightWallX, float InPlayer1LoseY, float InPlayer2LoseY)
{
	LeftWallX = InLeftWallX;
	RightWallX = InRightWallX;
	Player1LoseY = InPlayer1LoseY;
	Player2LoseY = InPlayer2LoseY;
}

/* Reflects the ball from left/right walls. */
void APongBall::CheckWallBounce()
{
	FVector Location = GetActorLocation();

	if (Location.X < LeftWallX)
	{
		Location.X = LeftWallX;
		Velocity.X = FMath::Abs(Velocity.X);
		SetActorLocation(Location);
	}
	else if (Location.X > RightWallX)
	{
		Location.X = RightWallX;
		Velocity.X = -FMath::Abs(Velocity.X);
		SetActorLocation(Location);
	}
}

/* Reflects the ball from both player paddles. */
void APongBall::CheckPaddleBounce()
{
	CheckOnePaddle(Player1Paddle, 1);
	CheckOnePaddle(Player2Paddle, -1);
}

/* Reflects the ball from one paddle if the overlap test succeeds. */
void APongBall::CheckOnePaddle(APongPaddle* Paddle, int32 DirectionY)
{
	if (Paddle == nullptr)
	{
		return;
	}

	const FVector BallLocation = GetActorLocation();
	const FVector PaddleLocation = Paddle->GetActorLocation();
	const float EffectiveHalfWidth = FMath::Max(20.0f, Paddle->GetHalfWidth() - PaddleCollisionShrink);
	const bool bCrossedPaddleY =
		(FMath::Min(PreviousLocation.Y, BallLocation.Y) <= PaddleLocation.Y + PaddleHitHalfHeight) &&
		(FMath::Max(PreviousLocation.Y, BallLocation.Y) >= PaddleLocation.Y - PaddleHitHalfHeight);
	const bool bMovingTowardPaddle =
		(DirectionY > 0 && Velocity.Y < 0.0f) ||
		(DirectionY < 0 && Velocity.Y > 0.0f);
	const bool bInsideX = FMath::Abs(BallLocation.X - PaddleLocation.X) <= EffectiveHalfWidth;

	if (!bCrossedPaddleY || !bMovingTowardPaddle || !bInsideX)
	{
		return;
	}

	FVector CorrectedLocation = BallLocation;
	CorrectedLocation.Y = PaddleLocation.Y + static_cast<float>(DirectionY) * PaddleHitHalfHeight;
	SetActorLocation(CorrectedLocation);

	Velocity.Y = FMath::Abs(Velocity.Y) * static_cast<float>(DirectionY);

	const float HitOffset = (BallLocation.X - PaddleLocation.X) / EffectiveHalfWidth;
	Velocity.X = FMath::Clamp(HitOffset, -1.0f, 1.0f);
	Velocity = Velocity.GetSafeNormal();
}

/* Applies bright material to the ball mesh. */
void APongBall::ApplyVisualMaterial()
{
	if (BaseVisualMaterial == nullptr || BallMesh == nullptr)
	{
		return;
	}

	UMaterialInstanceDynamic* DynamicMaterial = BallMesh->CreateDynamicMaterialInstance(0, BaseVisualMaterial);
	if (DynamicMaterial == nullptr)
	{
		return;
	}

	DynamicMaterial->SetVectorParameterValue(TEXT("Color"), BallColor);
	DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), BallColor);
	DynamicMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.18f);
	BallMesh->SetMaterial(0, DynamicMaterial);
}
