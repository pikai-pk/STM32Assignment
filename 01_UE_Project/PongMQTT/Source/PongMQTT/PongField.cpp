#include "PongField.h"
#include "PongBall.h"
#include "PongPaddle.h"
#include "Camera/CameraComponent.h"
#include "Components/ChildActorComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

APongField::APongField()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	FieldPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FieldPlane"));
	FieldPlane->SetupAttachment(SceneRoot);

	LeftWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftWall"));
	LeftWall->SetupAttachment(SceneRoot);

	RightWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightWall"));
	RightWall->SetupAttachment(SceneRoot);

	Player1LoseLine = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Player1LoseLine"));
	Player1LoseLine->SetupAttachment(SceneRoot);

	Player2LoseLine = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Player2LoseLine"));
	Player2LoseLine->SetupAttachment(SceneRoot);

	Player1PaddleComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("Player1Paddle"));
	Player1PaddleComponent->SetupAttachment(SceneRoot);
	Player1PaddleComponent->SetChildActorClass(APongPaddle::StaticClass());

	Player2PaddleComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("Player2Paddle"));
	Player2PaddleComponent->SetupAttachment(SceneRoot);
	Player2PaddleComponent->SetChildActorClass(APongPaddle::StaticClass());

	BallComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("Ball"));
	BallComponent->SetupAttachment(SceneRoot);
	BallComponent->SetChildActorClass(APongBall::StaticClass());

	FieldCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FieldCamera"));
	FieldCamera->SetupAttachment(SceneRoot);
	FieldCamera->ProjectionMode = ECameraProjectionMode::Orthographic;
	FieldCamera->bAutoActivate = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialAsset(TEXT("/Game/Material/BasicShapeMaterial.BasicShapeMaterial"));
	if (CubeMesh.Succeeded())
	{
		FieldPlane->SetStaticMesh(CubeMesh.Object);
		LeftWall->SetStaticMesh(CubeMesh.Object);
		RightWall->SetStaticMesh(CubeMesh.Object);
		Player1LoseLine->SetStaticMesh(CubeMesh.Object);
		Player2LoseLine->SetStaticMesh(CubeMesh.Object);
	}
	if (MaterialAsset.Succeeded())
	{
		BaseVisualMaterial = MaterialAsset.Object;
	}

	FieldPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftWall->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RightWall->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Player1LoseLine->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Player2LoseLine->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FieldPlane->SetCastShadow(false);
	LeftWall->SetCastShadow(false);
	RightWall->SetCastShadow(false);
	Player1LoseLine->SetCastShadow(false);
	Player2LoseLine->SetCastShadow(false);

	BuildVisualField();
}

/* Applies field dimensions to walls, paddles, ball, and camera when dragged or edited in the level. */
void APongField::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	BuildVisualField();
	ConfigureGeneratedActors();
}

/* Sets this field camera as the active editor/play camera when the level starts. */
void APongField::BeginPlay()
{
	Super::BeginPlay();

	ConfigureGeneratedActors();
	ApplyVisualMaterials();

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		PlayerController->SetViewTarget(this);
	}
}

/* Returns Player1 paddle spawned by the field. */
APongPaddle* APongField::GetPlayer1Paddle() const
{
	return GetChildActorAs<APongPaddle>(Player1PaddleComponent);
}

/* Returns Player2 paddle spawned by the field. */
APongPaddle* APongField::GetPlayer2Paddle() const
{
	return GetChildActorAs<APongPaddle>(Player2PaddleComponent);
}

/* Returns the ball spawned by the field. */
APongBall* APongField::GetBall() const
{
	return GetChildActorAs<APongBall>(BallComponent);
}

/* Updates mesh component scale and relative location using field dimensions. */
void APongField::BuildVisualField()
{
	const float VisualZ = -10.0f;

	FieldPlane->SetRelativeLocation(FVector(0.0f, 0.0f, VisualZ));
	FieldPlane->SetRelativeScale3D(FVector(HalfWidth * 2.0f / 100.0f, HalfHeight * 2.0f / 100.0f, 0.04f));

	LeftWall->SetRelativeLocation(FVector(-HalfWidth, 0.0f, 0.0f));
	LeftWall->SetRelativeScale3D(FVector(WallThickness / 100.0f, HalfHeight * 2.0f / 100.0f, 0.4f));

	RightWall->SetRelativeLocation(FVector(HalfWidth, 0.0f, 0.0f));
	RightWall->SetRelativeScale3D(FVector(WallThickness / 100.0f, HalfHeight * 2.0f / 100.0f, 0.4f));

	Player1LoseLine->SetRelativeLocation(FVector(0.0f, -HalfHeight, 8.0f));
	Player1LoseLine->SetRelativeScale3D(FVector(HalfWidth * 2.0f / 100.0f, 0.08f, 0.08f));

	Player2LoseLine->SetRelativeLocation(FVector(0.0f, HalfHeight, 8.0f));
	Player2LoseLine->SetRelativeScale3D(FVector(HalfWidth * 2.0f / 100.0f, 0.08f, 0.08f));

	Player1PaddleComponent->SetRelativeLocation(FVector(0.0f, -PaddleOffsetY, 30.0f));
	Player2PaddleComponent->SetRelativeLocation(FVector(0.0f, PaddleOffsetY, 30.0f));
	BallComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 35.0f));

	FieldCamera->SetRelativeLocation(FVector(0.0f, 0.0f, CameraHeight));
	FieldCamera->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	FieldCamera->OrthoWidth = CameraOrthoWidth;
}

/* Applies runtime material colors to the field meshes. */
void APongField::ApplyVisualMaterials()
{
	if (BaseVisualMaterial == nullptr)
	{
		return;
	}

	auto ApplyColor = [this](UStaticMeshComponent* Mesh, const FLinearColor& Color)
	{
		if (Mesh == nullptr)
		{
			return;
		}

		UMaterialInstanceDynamic* DynamicMaterial = Mesh->CreateDynamicMaterialInstance(0, BaseVisualMaterial);
		if (DynamicMaterial == nullptr)
		{
			return;
		}

		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
		DynamicMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.62f);
		Mesh->SetMaterial(0, DynamicMaterial);
	};

	ApplyColor(FieldPlane, FieldColor);
	ApplyColor(LeftWall, WallColor);
	ApplyColor(RightWall, WallColor);
	ApplyColor(Player1LoseLine, LoseLineColor);
	ApplyColor(Player2LoseLine, LoseLineColor);
}

/* Configures generated child actors with player ids and field bounds. */
void APongField::ConfigureGeneratedActors()
{
	if (APongPaddle* Player1Paddle = GetPlayer1Paddle())
	{
		Player1Paddle->SetDebugPaddleSizeEnabled(bDebugPaddleSize, DebugPaddleHalfWidth);
		const float Player1HalfWidth = Player1Paddle->GetHalfWidth();
		const float Player1MinX = -HalfWidth + WallThickness + Player1HalfWidth;
		const float Player1MaxX = HalfWidth - WallThickness - Player1HalfWidth;
		Player1Paddle->SetPlayerId(1);
		Player1Paddle->ConfigureMovementBounds(Player1MinX, Player1MaxX);
	}

	if (APongPaddle* Player2Paddle = GetPlayer2Paddle())
	{
		Player2Paddle->SetDebugPaddleSizeEnabled(bDebugPaddleSize, DebugPaddleHalfWidth);
		const float Player2HalfWidth = Player2Paddle->GetHalfWidth();
		const float Player2MinX = -HalfWidth + WallThickness + Player2HalfWidth;
		const float Player2MaxX = HalfWidth - WallThickness - Player2HalfWidth;
		Player2Paddle->SetPlayerId(2);
		Player2Paddle->ConfigureMovementBounds(Player2MinX, Player2MaxX);
	}

	if (APongBall* Ball = GetBall())
	{
		Ball->SetPaddles(GetPlayer1Paddle(), GetPlayer2Paddle());
		Ball->ConfigureFieldBounds(-HalfWidth + WallThickness, HalfWidth - WallThickness, -HalfHeight, HalfHeight);
	}
}
