#include "PongHUD.h"
#include "PongGameManager.h"
#include "PongHUDWidget.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

/* Creates the native UMG HUD when gameplay starts. */
void APongHUD::BeginPlay()
{
	Super::BeginPlay();

	bShowHUD = true;

	if (PongHudWidget != nullptr)
	{
		return;
	}

	PongHudWidget = CreateWidget<UPongHUDWidget>(GetOwningPlayerController(), UPongHUDWidget::StaticClass());
	if (PongHudWidget == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("PongHUD failed to create UPongHUDWidget."));
		return;
	}

	PongHudWidget->SetAnchorsInViewport(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
	PongHudWidget->SetAlignmentInViewport(FVector2D(0.0f, 0.0f));
	PongHudWidget->SetPositionInViewport(FVector2D::ZeroVector, false);
	PongHudWidget->SetDesiredSizeInViewport(FVector2D(1920.0f, 1080.0f));
	PongHudWidget->AddToViewport(10);
	UE_LOG(LogTemp, Log, TEXT("PongHUD added UPongHUDWidget to viewport."));
}

/* Draws a fallback border HUD so connection status is visible even if UMG layout is hidden. */
void APongHUD::DrawHUD()
{
	Super::DrawHUD();

	if (Canvas == nullptr)
	{
		return;
	}

	const float Width = Canvas->ClipX;
	const float Height = Canvas->ClipY;
	if (!bLoggedDrawHud)
	{
		bLoggedDrawHud = true;
		UE_LOG(LogTemp, Log, TEXT("PongHUD DrawHUD active. Canvas=%0.1fx%0.1f bShowHUD=%s"), Width, Height, bShowHUD ? TEXT("true") : TEXT("false"));
	}

	UFont* HudFont = GEngine != nullptr ? GEngine->GetMediumFont() : nullptr;
	const FLinearColor PanelColor(0.01f, 0.025f, 0.04f, 0.86f);
	const FLinearColor BorderColor(0.10f, 0.30f, 0.42f, 0.82f);
	const FLinearColor DarkLineColor(0.02f, 0.06f, 0.10f, 0.92f);
	const FLinearColor CyanColor(0.10f, 0.78f, 1.00f, 1.00f);
	const FLinearColor GreenColor(0.42f, 1.00f, 0.32f, 1.00f);
	const FLinearColor MutedColor(0.58f, 0.66f, 0.74f, 1.00f);

	DrawRect(BorderColor, 16.0f, 16.0f, Width - 32.0f, 68.0f);
	DrawRect(PanelColor, 18.0f, 18.0f, Width - 36.0f, 64.0f);
	DrawRect(DarkLineColor, 18.0f, 80.0f, Width - 36.0f, 3.0f);
	DrawText(TEXT("Pong"), FColor::White, 36.0f, 28.0f, HudFont, 2.05f, false);
	DrawText(TEXT("MQTT"), CyanColor.ToFColor(true), 144.0f, 28.0f, HudFont, 2.05f, false);

	APongGameManager* GameManager = GetOrCreateGameManager();
	const int32 ConnectedCount = GameManager != nullptr ? GameManager->GetConnectedBoardCount() : 0;
	const FString TopStatus = FString::Printf(TEXT("Boards %d/2    %s"), ConnectedCount, ConnectedCount >= 2 ? TEXT("Ready") : TEXT("Waiting for MQTT"));
	DrawText(TopStatus, ConnectedCount >= 2 ? GreenColor.ToFColor(true) : MutedColor.ToFColor(true), 330.0f, 38.0f, HudFont, 1.45f, false);

	DrawPlayerCard(1, 20.0f, 108.0f, 250.0f, 280.0f, CyanColor);
	DrawPlayerCard(2, Width - 270.0f, 108.0f, 250.0f, 280.0f, GreenColor);

	const float LogWidth = FMath::Min(680.0f, Width - 40.0f);
	DrawRect(BorderColor, 20.0f, Height - 118.0f, LogWidth, 98.0f);
	DrawRect(PanelColor, 22.0f, Height - 116.0f, LogWidth - 4.0f, 94.0f);
	DrawText(TEXT("CONNECTION LOG"), CyanColor.ToFColor(true), 40.0f, Height - 104.0f, HudFont, 1.25f, false);

	FString LogLine = TEXT("Waiting for STM32 status messages...");
	if (GameManager != nullptr)
	{
		LogLine = FString::Printf(TEXT("P1 IP: %s    P2 IP: %s"), *GameManager->GetPlayerIp(1), *GameManager->GetPlayerIp(2));
	}
	DrawText(LogLine, MutedColor.ToFColor(true), 40.0f, Height - 66.0f, HudFont, 1.15f, false);

	if (GameManager != nullptr && GameManager->IsCountdownRunning())
	{
		DrawRect(BorderColor, Width * 0.5f - 150.0f, Height * 0.5f - 115.0f, 300.0f, 230.0f);
		DrawRect(FLinearColor(0.01f, 0.02f, 0.03f, 0.92f), Width * 0.5f - 146.0f, Height * 0.5f - 111.0f, 292.0f, 222.0f);
		DrawText(TEXT("COUNTDOWN"), CyanColor.ToFColor(true), Width * 0.5f - 116.0f, Height * 0.5f - 84.0f, HudFont, 1.45f, false);
		DrawText(FString::FromInt(GameManager->GetCountdownSeconds()), FColor::White, Width * 0.5f - 38.0f, Height * 0.5f - 30.0f, HudFont, 4.35f, false);
	}

	if (GameManager != nullptr && GameManager->IsGameOver())
	{
		DrawRect(BorderColor, Width - 370.0f, Height - 250.0f, 350.0f, 138.0f);
		DrawRect(FLinearColor(0.01f, 0.02f, 0.03f, 0.92f), Width - 366.0f, Height - 246.0f, 342.0f, 130.0f);
		DrawText(TEXT("GAME OVER"), FColor::White, Width - 258.0f, Height - 230.0f, HudFont, 1.35f, false);
		DrawText(FString::Printf(TEXT("Winner: Player %d"), GameManager->GetWinnerPlayerId()), GreenColor.ToFColor(true), Width - 336.0f, Height - 184.0f, HudFont, 1.25f, false);
		DrawText(FString::Printf(TEXT("Loser:  Player %d"), GameManager->GetLoserPlayerId()), CyanColor.ToFColor(true), Width - 336.0f, Height - 148.0f, HudFont, 1.25f, false);
	}
}

/* Draws one STM32 player status card on the screen border. */
void APongHUD::DrawPlayerCard(int32 PlayerId, float X, float Y, float Width, float Height, const FLinearColor& AccentColor)
{
	const FLinearColor PanelColor(0.01f, 0.02f, 0.03f, 0.78f);
	const FLinearColor BorderColor(0.10f, 0.30f, 0.42f, 0.82f);
	const FLinearColor InnerBoxColor(0.02f, 0.06f, 0.09f, 0.88f);
	const FLinearColor GreenColor(0.42f, 1.00f, 0.32f, 1.00f);
	const FLinearColor MutedColor(0.58f, 0.66f, 0.74f, 1.00f);
	UFont* HudFont = GEngine != nullptr ? GEngine->GetMediumFont() : nullptr;
	APongGameManager* GameManager = GetOrCreateGameManager();
	const FString Ip = GameManager != nullptr ? GameManager->GetPlayerIp(PlayerId) : TEXT("Unknown");
	const bool bOnline = !Ip.Equals(TEXT("Unknown"), ESearchCase::IgnoreCase) && !Ip.IsEmpty();

	DrawRect(BorderColor, X, Y, Width, Height);
	DrawRect(PanelColor, X + 2.0f, Y + 2.0f, Width - 4.0f, Height - 4.0f);
	DrawRect(AccentColor.CopyWithNewOpacity(0.92f), X + 2.0f, Y + 2.0f, 5.0f, Height - 4.0f);
	DrawText(FString::Printf(TEXT("PLAYER %d"), PlayerId), AccentColor.ToFColor(true), X + 26.0f, Y + 20.0f, HudFont, 1.45f, false);
	DrawText(bOnline ? TEXT("ONLINE") : TEXT("WAITING"), bOnline ? GreenColor.ToFColor(true) : MutedColor.ToFColor(true), X + 26.0f, Y + 70.0f, HudFont, 1.2f, false);

	DrawText(TEXT("IP"), MutedColor.ToFColor(true), X + 26.0f, Y + 112.0f, HudFont, 1.05f, false);
	DrawRect(InnerBoxColor, X + 24.0f, Y + 138.0f, Width - 48.0f, 34.0f);
	DrawText(Ip, AccentColor.ToFColor(true), X + 36.0f, Y + 142.0f, HudFont, 1.05f, false);

	DrawText(TEXT("TOPIC"), MutedColor.ToFColor(true), X + 26.0f, Y + 190.0f, HudFont, 1.05f, false);
	DrawRect(InnerBoxColor, X + 24.0f, Y + 216.0f, Width - 48.0f, 34.0f);
	DrawText(FString::Printf(TEXT("pong/player%d/#"), PlayerId), AccentColor.ToFColor(true), X + 36.0f, Y + 220.0f, HudFont, 0.92f, false);
}

/* Finds or creates the game manager used by HUD, commands, and MQTT simulation. */
APongGameManager* APongHUD::GetOrCreateGameManager()
{
	if (CachedGameManager != nullptr)
	{
		return CachedGameManager;
	}

	CachedGameManager = Cast<APongGameManager>(UGameplayStatics::GetActorOfClass(this, APongGameManager::StaticClass()));
	if (CachedGameManager != nullptr)
	{
		return CachedGameManager;
	}

	if (GetWorld() == nullptr)
	{
		return nullptr;
	}

	CachedGameManager = GetWorld()->SpawnActor<APongGameManager>(APongGameManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
	if (CachedGameManager != nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("PongHUD spawned PongGameManager automatically."));
	}

	return CachedGameManager;
}
