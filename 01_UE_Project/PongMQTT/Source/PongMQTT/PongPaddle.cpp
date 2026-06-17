#include "PongPaddle.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

APongPaddle::APongPaddle()
{
	PrimaryActorTick.bCanEverTick = true;

	PaddleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PaddleMesh"));
	SetRootComponent(PaddleMesh);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialAsset(TEXT("/Game/Material/BasicShapeMaterial.BasicShapeMaterial"));
	if (CubeMesh.Succeeded())
	{
		PaddleMesh->SetStaticMesh(CubeMesh.Object);
	}
	if (MaterialAsset.Succeeded())
	{
		BaseVisualMaterial = MaterialAsset.Object;
	}

	PaddleMesh->SetWorldScale3D(FVector(3.2f, 0.28f, 0.18f));
	PaddleMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PaddleMesh->SetCastShadow(false);
}

/* Applies debug paddle settings when values are changed in the editor Details panel. */
void APongPaddle::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyDebugPaddleSettings();
}

/* Caches the starting transform after the actor is placed in the level. */
void APongPaddle::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();
	ApplyDebugPaddleSettings();
	ApplyVisualMaterial();
}

/* Updates the paddle position every frame using the current move input. */
void APongPaddle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Location = GetActorLocation();
	Location.X = FMath::Clamp(Location.X + MoveInput * MoveSpeed * DeltaTime, MinX, MaxX);
	SetActorLocation(Location);
}

/* Sets which player this paddle belongs to. Player1 is bottom, Player2 is top. */
void APongPaddle::SetPlayerId(int32 InPlayerId)
{
	PlayerId = InPlayerId;
	if (HasActorBegunPlay())
	{
		ApplyVisualMaterial();
	}
}

/* Returns this paddle's player id. */
int32 APongPaddle::GetPlayerId() const
{
	return PlayerId;
}

/* Sets left/right movement input. Expected range is -1..1. */
void APongPaddle::SetMoveInput(float InMoveInput)
{
	MoveInput = FMath::Clamp(InMoveInput, -1.0f, 1.0f);
}

/* Moves the paddle by one fixed input step, used by STM32 button press events. */
void APongPaddle::NudgeByInput(float InMoveInput)
{
	const float Direction = FMath::Clamp(InMoveInput, -1.0f, 1.0f);
	if (FMath::IsNearlyZero(Direction))
	{
		return;
	}

	FVector Location = GetActorLocation();
	Location.X = FMath::Clamp(Location.X + Direction * ButtonStepDistance, MinX, MaxX);
	SetActorLocation(Location);
	MoveInput = 0.0f;
}

/* Returns the current movement input. */
float APongPaddle::GetMoveInput() const
{
	return MoveInput;
}

/* Returns the half width used by ball collision checks. */
float APongPaddle::GetHalfWidth() const
{
	return HalfWidth;
}

/* Configures horizontal movement limits so the paddle cannot cross the side walls. */
void APongPaddle::ConfigureMovementBounds(float InMinX, float InMaxX)
{
	MinX = InMinX;
	MaxX = InMaxX;
}

/* Enables or disables debug paddle length from PongField or Blueprint. */
void APongPaddle::SetDebugPaddleSizeEnabled(bool bEnabled, float InDebugHalfWidth)
{
	bDebugPaddleSize = bEnabled;
	DebugHalfWidth = InDebugHalfWidth;
	ApplyDebugPaddleSettings();
}

/* Resets the paddle to its configured start location and clears input. */
void APongPaddle::ResetPaddle()
{
	SetActorLocation(StartLocation);
	MoveInput = 0.0f;
	ApplyDebugPaddleSettings();
}

/* Updates the paddle mesh length. Debug mode uses DebugHalfWidth; normal mode restores DefaultHalfWidth. */
void APongPaddle::ApplyDebugPaddleSettings()
{
	if (bDebugPaddleSize)
	{
		HalfWidth = DebugHalfWidth;
	}
	else
	{
		HalfWidth = DefaultHalfWidth;
	}

	if (PaddleMesh != nullptr)
	{
		FVector NewScale = DefaultMeshScale;
		NewScale.X = (HalfWidth * 2.0f) / 100.0f;
		PaddleMesh->SetRelativeScale3D(NewScale);
	}
}

/* Applies player-colored material to the paddle mesh. */
void APongPaddle::ApplyVisualMaterial()
{
	if (BaseVisualMaterial == nullptr || PaddleMesh == nullptr)
	{
		return;
	}

	const FLinearColor Color = PlayerId == 1 ? Player1Color : Player2Color;
	UMaterialInstanceDynamic* DynamicMaterial = PaddleMesh->CreateDynamicMaterialInstance(0, BaseVisualMaterial);
	if (DynamicMaterial == nullptr)
	{
		return;
	}

	DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
	DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
	DynamicMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.35f);
	PaddleMesh->SetMaterial(0, DynamicMaterial);
}
