#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PongHUD.generated.h"

class UPongHUDWidget;
class APongGameManager;

UCLASS()
class PONGMQTT_API APongHUD : public AHUD
{
	GENERATED_BODY()

protected:
	/* Creates the native UMG HUD when gameplay starts. */
	virtual void BeginPlay() override;

	/* Draws a fallback border HUD so connection status is visible even if UMG layout is hidden. */
	virtual void DrawHUD() override;

private:
	/* Finds or creates the game manager used by HUD, commands, and MQTT simulation. */
	APongGameManager* GetOrCreateGameManager();

	/* Draws one STM32 player status card on the screen border. */
	void DrawPlayerCard(int32 PlayerId, float X, float Y, float Width, float Height, const FLinearColor& AccentColor);

	UPROPERTY()
	UPongHUDWidget* PongHudWidget = nullptr;

	UPROPERTY()
	APongGameManager* CachedGameManager = nullptr;

	bool bLoggedDrawHud = false;
};
