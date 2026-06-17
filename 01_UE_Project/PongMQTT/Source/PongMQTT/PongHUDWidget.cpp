#include "PongHUDWidget.h"
#include "PongGameManager.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	const FLinearColor PanelColor(0.015f, 0.025f, 0.035f, 0.84f);
	const FLinearColor CyanColor(0.10f, 0.78f, 1.00f, 1.00f);
	const FLinearColor GreenColor(0.42f, 1.00f, 0.32f, 1.00f);
	const FLinearColor MutedColor(0.58f, 0.66f, 0.74f, 1.00f);
	const FLinearColor WhiteColor(0.94f, 0.97f, 1.00f, 1.00f);
	const FLinearColor OfflineColor(0.22f, 0.27f, 0.31f, 1.00f);

	UTextBlock* MakeText(UWidgetTree* Tree, const FString& Name, const FString& Value, int32 FontSize, const FLinearColor& Color)
	{
		UTextBlock* Text = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), FName(*Name));
		Text->SetText(FText::FromString(Value));
		Text->SetColorAndOpacity(FSlateColor(Color));
		Text->SetJustification(ETextJustify::Left);
		Text->SetAutoWrapText(false);
		FSlateFontInfo Font = Text->GetFont();
		Font.Size = FontSize;
		Font.TypefaceFontName = TEXT("Bold");
		Text->SetFont(Font);
		return Text;
	}

	UBorder* MakePanel(UWidgetTree* Tree, const FString& Name, const FLinearColor& Color = PanelColor)
	{
		UBorder* Border = Tree->ConstructWidget<UBorder>(UBorder::StaticClass(), FName(*Name));
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::Box;
		Brush.TintColor = FSlateColor(Color);
		Border->SetBrush(Brush);
		Border->SetBrushColor(Color);
		Border->SetPadding(FMargin(18.0f, 14.0f));
		return Border;
	}

	UCanvasPanelSlot* AddToCanvas(UCanvasPanel* Canvas, UWidget* Widget, const FVector2D& Position, const FVector2D& Size, const FAnchors& Anchors = FAnchors(0.0f, 0.0f))
	{
		UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Widget);
		Slot->SetAnchors(Anchors);
		Slot->SetAlignment(FVector2D(0.0f, 0.0f));
		Slot->SetPosition(Position);
		Slot->SetSize(Size);
		return Slot;
	}

	void AddSpacer(UWidgetTree* Tree, UVerticalBox* Box, float Height)
	{
		USpacer* Spacer = Tree->ConstructWidget<USpacer>(USpacer::StaticClass());
		Spacer->SetSize(FVector2D(1.0f, Height));
		Box->AddChildToVerticalBox(Spacer);
	}
}

void UPongHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!bBuiltHud)
	{
		BuildHud();
		bBuiltHud = true;
		UE_LOG(LogTemp, Log, TEXT("PongHUDWidget built native UMG layout."));
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	TryBindGameManager();
	RefreshAll();
}

void UPongHUDWidget::NativeDestruct()
{
	if (GameManager != nullptr)
	{
		GameManager->OnBoardCountChanged.RemoveDynamic(this, &UPongHUDWidget::HandleBoardCountChanged);
		GameManager->OnCountdownChanged.RemoveDynamic(this, &UPongHUDWidget::HandleCountdownChanged);
		GameManager->OnMatchStarted.RemoveDynamic(this, &UPongHUDWidget::HandleMatchStarted);
		GameManager->OnMatchEnded.RemoveDynamic(this, &UPongHUDWidget::HandleMatchEnded);
	}

	Super::NativeDestruct();
}

void UPongHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (GameManager == nullptr)
	{
		TryBindGameManager();
		RefreshAll();
	}
}

void UPongHUDWidget::BuildHud()
{
	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	BuildTopBar(RootCanvas);
	BuildCenterPanels(RootCanvas);
	BuildPlayerCard(RootCanvas, 1, FVector2D(16.0f, 122.0f));
	BuildPlayerCard(RootCanvas, 2, FVector2D(1456.0f, 122.0f));
}

void UPongHUDWidget::BuildTopBar(UCanvasPanel* RootCanvas)
{
	UBorder* TopBar = MakePanel(WidgetTree, TEXT("TopBar"), FLinearColor(0.01f, 0.02f, 0.03f, 0.90f));
	TopBar->SetPadding(FMargin(30.0f, 12.0f));
	UCanvasPanelSlot* TopBarSlot = AddToCanvas(RootCanvas, TopBar, FVector2D(16.0f, 16.0f), FVector2D(-32.0f, 78.0f), FAnchors(0.0f, 0.0f, 1.0f, 0.0f));
	TopBarSlot->SetOffsets(FMargin(16.0f, 16.0f, 16.0f, 78.0f));

	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("TopBarRow"));
	TopBar->SetContent(Row);

	UTextBlock* TitleText = MakeText(WidgetTree, TEXT("TitleText"), TEXT("Pong"), 34, WhiteColor);
	Row->AddChildToHorizontalBox(TitleText);

	UTextBlock* MqttText = MakeText(WidgetTree, TEXT("TitleMqttText"), TEXT("MQTT"), 34, CyanColor);
	Row->AddChildToHorizontalBox(MqttText);

	USpacer* TitleSpacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass());
	TitleSpacer->SetSize(FVector2D(62.0f, 1.0f));
	Row->AddChildToHorizontalBox(TitleSpacer);

	BoardCountText = MakeText(WidgetTree, TEXT("BoardCountText"), TEXT("Boards 0/2"), 20, WhiteColor);
	Row->AddChildToHorizontalBox(BoardCountText);

	USpacer* FillSpacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass());
	UHorizontalBoxSlot* FillSlot = Row->AddChildToHorizontalBox(FillSpacer);
	FillSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

	MqttStateText = MakeText(WidgetTree, TEXT("MqttStateText"), TEXT("Waiting"), 20, GreenColor);
	Row->AddChildToHorizontalBox(MqttStateText);
}

void UPongHUDWidget::BuildPlayerCard(UCanvasPanel* RootCanvas, int32 PlayerId, const FVector2D& Position)
{
	const bool bPlayerOne = PlayerId == 1;
	const FLinearColor AccentColor = bPlayerOne ? CyanColor : GreenColor;

	UBorder* Card = MakePanel(WidgetTree, bPlayerOne ? TEXT("Player1Card") : TEXT("Player2Card"));
	UCanvasPanelSlot* CardSlot = AddToCanvas(RootCanvas, Card, Position, FVector2D(222.0f, 470.0f), bPlayerOne ? FAnchors(0.0f, 0.0f) : FAnchors(1.0f, 0.0f));
	if (!bPlayerOne)
	{
		CardSlot->SetAlignment(FVector2D(1.0f, 0.0f));
		CardSlot->SetPosition(FVector2D(-16.0f, Position.Y));
	}

	UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), bPlayerOne ? TEXT("Player1Column") : TEXT("Player2Column"));
	Card->SetContent(Column);

	UTextBlock* NameText = MakeText(WidgetTree, bPlayerOne ? TEXT("Player1NameText") : TEXT("Player2NameText"), FString::Printf(TEXT("PLAYER %d"), PlayerId), 23, AccentColor);
	NameText->SetJustification(ETextJustify::Center);
	Column->AddChildToVerticalBox(NameText);

	AddSpacer(WidgetTree, Column, 18.0f);

	UHorizontalBox* StatusRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), bPlayerOne ? TEXT("Player1StatusRow") : TEXT("Player2StatusRow"));
	Column->AddChildToVerticalBox(StatusRow);

	UBorder* Dot = MakePanel(WidgetTree, bPlayerOne ? TEXT("Player1Dot") : TEXT("Player2Dot"), OfflineColor);
	Dot->SetPadding(FMargin(0.0f));
	USizeBox* DotSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), bPlayerOne ? TEXT("Player1DotSize") : TEXT("Player2DotSize"));
	DotSize->SetWidthOverride(16.0f);
	DotSize->SetHeightOverride(16.0f);
	DotSize->SetContent(Dot);
	StatusRow->AddChildToHorizontalBox(DotSize);

	UTextBlock* StatusText = MakeText(WidgetTree, bPlayerOne ? TEXT("Player1StatusText") : TEXT("Player2StatusText"), TEXT("Waiting"), 18, MutedColor);
	UHorizontalBoxSlot* StatusTextSlot = StatusRow->AddChildToHorizontalBox(StatusText);
	StatusTextSlot->SetPadding(FMargin(10.0f, -3.0f, 0.0f, 0.0f));

	AddSpacer(WidgetTree, Column, 26.0f);
	Column->AddChildToVerticalBox(MakeText(WidgetTree, bPlayerOne ? TEXT("Player1IpLabel") : TEXT("Player2IpLabel"), TEXT("IP"), 16, MutedColor));

	UTextBlock* IpText = MakeText(WidgetTree, bPlayerOne ? TEXT("Player1IpText") : TEXT("Player2IpText"), TEXT("Unknown"), 18, AccentColor);
	UBorder* IpBox = MakePanel(WidgetTree, bPlayerOne ? TEXT("Player1IpBox") : TEXT("Player2IpBox"), FLinearColor(0.02f, 0.04f, 0.06f, 0.74f));
	IpBox->SetPadding(FMargin(12.0f, 8.0f));
	IpBox->SetContent(IpText);
	Column->AddChildToVerticalBox(IpBox);

	AddSpacer(WidgetTree, Column, 26.0f);
	Column->AddChildToVerticalBox(MakeText(WidgetTree, bPlayerOne ? TEXT("Player1TopicLabel") : TEXT("Player2TopicLabel"), TEXT("TOPIC (PUB/SUB)"), 15, MutedColor));

	UBorder* TopicBox = MakePanel(WidgetTree, bPlayerOne ? TEXT("Player1TopicBox") : TEXT("Player2TopicBox"), FLinearColor(0.02f, 0.04f, 0.06f, 0.74f));
	TopicBox->SetPadding(FMargin(12.0f, 8.0f));
	TopicBox->SetContent(MakeText(WidgetTree, bPlayerOne ? TEXT("Player1TopicText") : TEXT("Player2TopicText"), FString::Printf(TEXT("pong/player%d/#"), PlayerId), 15, AccentColor));
	Column->AddChildToVerticalBox(TopicBox);

	AddSpacer(WidgetTree, Column, 36.0f);
	UTextBlock* ChipText = MakeText(WidgetTree, bPlayerOne ? TEXT("Player1ChipText") : TEXT("Player2ChipText"), TEXT("[ STM32F407VETx ]"), 16, AccentColor);
	ChipText->SetJustification(ETextJustify::Center);
	Column->AddChildToVerticalBox(ChipText);

	if (bPlayerOne)
	{
		Player1Dot = Dot;
		Player1StatusText = StatusText;
		Player1IpText = IpText;
	}
	else
	{
		Player2Dot = Dot;
		Player2StatusText = StatusText;
		Player2IpText = IpText;
	}
}

void UPongHUDWidget::BuildCenterPanels(UCanvasPanel* RootCanvas)
{
	CountdownPanel = MakePanel(WidgetTree, TEXT("CountdownPanel"), FLinearColor(0.02f, 0.035f, 0.05f, 0.92f));
	UCanvasPanelSlot* CountdownSlot = AddToCanvas(RootCanvas, CountdownPanel, FVector2D(0.0f, -80.0f), FVector2D(420.0f, 300.0f), FAnchors(0.5f, 0.5f));
	CountdownSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	CountdownPanel->SetVisibility(ESlateVisibility::Collapsed);

	UVerticalBox* CountdownColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CountdownColumn"));
	CountdownPanel->SetContent(CountdownColumn);
	UTextBlock* CountdownLabel = MakeText(WidgetTree, TEXT("CountdownLabel"), TEXT("COUNTDOWN"), 24, CyanColor);
	CountdownLabel->SetJustification(ETextJustify::Center);
	CountdownColumn->AddChildToVerticalBox(CountdownLabel);
	AddSpacer(WidgetTree, CountdownColumn, 22.0f);
	CountdownText = MakeText(WidgetTree, TEXT("CountdownText"), TEXT("5"), 112, WhiteColor);
	CountdownText->SetJustification(ETextJustify::Center);
	CountdownColumn->AddChildToVerticalBox(CountdownText);

	ResultPanel = MakePanel(WidgetTree, TEXT("ResultPanel"), FLinearColor(0.02f, 0.035f, 0.05f, 0.92f));
	UCanvasPanelSlot* ResultSlot = AddToCanvas(RootCanvas, ResultPanel, FVector2D(-16.0f, -120.0f), FVector2D(402.0f, 208.0f), FAnchors(1.0f, 1.0f));
	ResultSlot->SetAlignment(FVector2D(1.0f, 1.0f));
	ResultPanel->SetVisibility(ESlateVisibility::Collapsed);

	UVerticalBox* ResultColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ResultColumn"));
	ResultPanel->SetContent(ResultColumn);
	UTextBlock* ResultTitle = MakeText(WidgetTree, TEXT("ResultTitle"), TEXT("GAME OVER"), 22, WhiteColor);
	ResultTitle->SetJustification(ETextJustify::Center);
	ResultColumn->AddChildToVerticalBox(ResultTitle);
	AddSpacer(WidgetTree, ResultColumn, 22.0f);

	UHorizontalBox* ResultRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ResultRow"));
	ResultColumn->AddChildToVerticalBox(ResultRow);
	WinnerText = MakeText(WidgetTree, TEXT("WinnerText"), TEXT("WINNER\nPLAYER -"), 18, GreenColor);
	LoserText = MakeText(WidgetTree, TEXT("LoserText"), TEXT("LOSER\nPLAYER -"), 18, CyanColor);
	ResultRow->AddChildToHorizontalBox(WinnerText);
	UHorizontalBoxSlot* LoserSlot = ResultRow->AddChildToHorizontalBox(LoserText);
	LoserSlot->SetPadding(FMargin(70.0f, 0.0f, 0.0f, 0.0f));

	UBorder* LogPanel = MakePanel(WidgetTree, TEXT("ConnectionLogPanel"), FLinearColor(0.01f, 0.025f, 0.035f, 0.82f));
	UCanvasPanelSlot* LogSlotPanel = AddToCanvas(RootCanvas, LogPanel, FVector2D(16.0f, -16.0f), FVector2D(620.0f, 118.0f), FAnchors(0.0f, 1.0f));
	LogSlotPanel->SetAlignment(FVector2D(0.0f, 1.0f));
	UVerticalBox* LogColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LogColumn"));
	LogPanel->SetContent(LogColumn);
	LogColumn->AddChildToVerticalBox(MakeText(WidgetTree, TEXT("ConnectionLogTitle"), TEXT("CONNECTION LOG"), 16, CyanColor));
	ConnectionLogText = MakeText(WidgetTree, TEXT("ConnectionLogText"), TEXT("Waiting for STM32 boards..."), 15, MutedColor);
	UVerticalBoxSlot* LogSlot = LogColumn->AddChildToVerticalBox(ConnectionLogText);
	LogSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 0.0f));
}

void UPongHUDWidget::TryBindGameManager()
{
	if (GameManager != nullptr)
	{
		return;
	}

	GameManager = Cast<APongGameManager>(UGameplayStatics::GetActorOfClass(this, APongGameManager::StaticClass()));
	if (GameManager == nullptr)
	{
		return;
	}

	GameManager->OnBoardCountChanged.AddUniqueDynamic(this, &UPongHUDWidget::HandleBoardCountChanged);
	GameManager->OnCountdownChanged.AddUniqueDynamic(this, &UPongHUDWidget::HandleCountdownChanged);
	GameManager->OnMatchStarted.AddUniqueDynamic(this, &UPongHUDWidget::HandleMatchStarted);
	GameManager->OnMatchEnded.AddUniqueDynamic(this, &UPongHUDWidget::HandleMatchEnded);
}

void UPongHUDWidget::RefreshAll()
{
	if (GameManager == nullptr)
	{
		if (BoardCountText != nullptr)
		{
			BoardCountText->SetText(FText::FromString(TEXT("Boards 0/2")));
		}
		if (MqttStateText != nullptr)
		{
			MqttStateText->SetText(FText::FromString(TEXT("Waiting")));
			MqttStateText->SetColorAndOpacity(FSlateColor(MutedColor));
		}
		if (CountdownPanel != nullptr)
		{
			CountdownPanel->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (ResultPanel != nullptr)
		{
			ResultPanel->SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	const int32 Count = GameManager->GetConnectedBoardCount();
	if (BoardCountText != nullptr)
	{
		BoardCountText->SetText(FText::FromString(FString::Printf(TEXT("Boards %d/2"), Count)));
	}

	if (MqttStateText != nullptr)
	{
		MqttStateText->SetText(FText::FromString(Count >= 2 ? TEXT("Ready") : TEXT("Waiting")));
		MqttStateText->SetColorAndOpacity(FSlateColor(Count >= 2 ? GreenColor : MutedColor));
	}

	RefreshPlayerCard(1);
	RefreshPlayerCard(2);

	if (CountdownPanel != nullptr)
	{
		CountdownPanel->SetVisibility(GameManager->IsCountdownRunning() ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (CountdownText != nullptr)
	{
		CountdownText->SetText(FText::AsNumber(GameManager->GetCountdownSeconds()));
	}

	if (ResultPanel != nullptr)
	{
		ResultPanel->SetVisibility(GameManager->IsGameOver() ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (WinnerText != nullptr)
	{
		WinnerText->SetText(FText::FromString(FString::Printf(TEXT("WINNER\nPLAYER %d"), GameManager->GetWinnerPlayerId())));
	}
	if (LoserText != nullptr)
	{
		LoserText->SetText(FText::FromString(FString::Printf(TEXT("LOSER\nPLAYER %d"), GameManager->GetLoserPlayerId())));
	}

	if (ConnectionLogText != nullptr)
	{
		const FString Player1Ip = GameManager->GetPlayerIp(1);
		const FString Player2Ip = GameManager->GetPlayerIp(2);
		ConnectionLogText->SetText(FText::FromString(FString::Printf(TEXT("Player 1 IP: %s\nPlayer 2 IP: %s\n%s"),
			*Player1Ip,
			*Player2Ip,
			Count >= 2 ? TEXT("All boards connected. Game starting...") : TEXT("Waiting for STM32 status messages..."))));
	}
}

void UPongHUDWidget::RefreshPlayerCard(int32 PlayerId)
{
	if (GameManager == nullptr)
	{
		return;
	}

	const FString Ip = GameManager->GetPlayerIp(PlayerId);
	const bool bOnline = !Ip.Equals(TEXT("Unknown"), ESearchCase::IgnoreCase) && !Ip.IsEmpty();
	UBorder* Dot = PlayerId == 1 ? Player1Dot : Player2Dot;
	UTextBlock* StatusText = PlayerId == 1 ? Player1StatusText : Player2StatusText;
	UTextBlock* IpText = PlayerId == 1 ? Player1IpText : Player2IpText;
	const FLinearColor AccentColor = PlayerId == 1 ? CyanColor : GreenColor;

	if (Dot != nullptr)
	{
		Dot->SetBrushColor(bOnline ? GreenColor : OfflineColor);
	}
	if (StatusText != nullptr)
	{
		StatusText->SetText(FText::FromString(bOnline ? TEXT("Online") : TEXT("Waiting")));
		StatusText->SetColorAndOpacity(FSlateColor(bOnline ? GreenColor : MutedColor));
	}
	if (IpText != nullptr)
	{
		IpText->SetText(FText::FromString(Ip));
		IpText->SetColorAndOpacity(FSlateColor(bOnline ? AccentColor : MutedColor));
	}
}

void UPongHUDWidget::HandleBoardCountChanged(int32 ConnectedBoardCount)
{
	RefreshAll();
}

void UPongHUDWidget::HandleCountdownChanged(int32 CountdownSeconds)
{
	RefreshAll();
}

void UPongHUDWidget::HandleMatchStarted()
{
	RefreshAll();
}

void UPongHUDWidget::HandleMatchEnded(int32 WinnerPlayerId, int32 LoserPlayerId, const FString& WinnerIp, const FString& LoserIp)
{
	RefreshAll();
}
