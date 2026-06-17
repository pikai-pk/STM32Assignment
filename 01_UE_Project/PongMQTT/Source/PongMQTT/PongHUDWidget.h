#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PongHUDWidget.generated.h"

class APongGameManager;
class UBorder;
class UCanvasPanel;
class UTextBlock;

UCLASS()
class PONGMQTT_API UPongHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* Builds the Pong MQTT HUD and binds it to the game manager. */
	virtual void NativeConstruct() override;

	/* Removes event bindings when the widget is destroyed. */
	virtual void NativeDestruct() override;

protected:
	/* Keeps trying to resolve the manager if actor begin-play order is different in the level. */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	/* Creates the visual tree that matches the WidgetUI_Reference style. */
	void BuildHud();

	/* Creates the top title and connection status bar. */
	void BuildTopBar(UCanvasPanel* RootCanvas);

	/* Creates one STM32 player connection card. */
	void BuildPlayerCard(UCanvasPanel* RootCanvas, int32 PlayerId, const FVector2D& Position);

	/* Creates the field frame, countdown panel, result panel, and connection log. */
	void BuildCenterPanels(UCanvasPanel* RootCanvas);

	/* Finds APongGameManager and binds UI refresh events. */
	void TryBindGameManager();

	/* Refreshes all visible text and panel states from APongGameManager. */
	void RefreshAll();

	/* Refreshes one player card from the latest IP and online state. */
	void RefreshPlayerCard(int32 PlayerId);

	UFUNCTION()
	void HandleBoardCountChanged(int32 ConnectedBoardCount);

	UFUNCTION()
	void HandleCountdownChanged(int32 CountdownSeconds);

	UFUNCTION()
	void HandleMatchStarted();

	UFUNCTION()
	void HandleMatchEnded(int32 WinnerPlayerId, int32 LoserPlayerId, const FString& WinnerIp, const FString& LoserIp);

	UPROPERTY()
	APongGameManager* GameManager = nullptr;

	UPROPERTY()
	UTextBlock* BoardCountText = nullptr;

	UPROPERTY()
	UTextBlock* MqttStateText = nullptr;

	UPROPERTY()
	UTextBlock* Player1StatusText = nullptr;

	UPROPERTY()
	UTextBlock* Player1IpText = nullptr;

	UPROPERTY()
	UBorder* Player1Dot = nullptr;

	UPROPERTY()
	UTextBlock* Player2StatusText = nullptr;

	UPROPERTY()
	UTextBlock* Player2IpText = nullptr;

	UPROPERTY()
	UBorder* Player2Dot = nullptr;

	UPROPERTY()
	UBorder* CountdownPanel = nullptr;

	UPROPERTY()
	UTextBlock* CountdownText = nullptr;

	UPROPERTY()
	UBorder* ResultPanel = nullptr;

	UPROPERTY()
	UTextBlock* WinnerText = nullptr;

	UPROPERTY()
	UTextBlock* LoserText = nullptr;

	UPROPERTY()
	UTextBlock* ConnectionLogText = nullptr;

	bool bBuiltHud = false;
};
