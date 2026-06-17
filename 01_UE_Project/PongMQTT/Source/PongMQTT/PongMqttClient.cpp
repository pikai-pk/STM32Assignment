#include "PongMqttClient.h"
#include "PongGameManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Json.h"
#include "SocketSubsystem.h"
#include "Sockets.h"

namespace
{
	void AppendUtf8String(TArray<uint8>& OutBytes, const FString& Text)
	{
		FTCHARToUTF8 Utf8(*Text);
		const int32 Len = Utf8.Length();
		OutBytes.Add(static_cast<uint8>((Len >> 8) & 0xFF));
		OutBytes.Add(static_cast<uint8>(Len & 0xFF));
		OutBytes.Append(reinterpret_cast<const uint8*>(Utf8.Get()), Len);
	}

	void AppendRemainingLength(TArray<uint8>& OutBytes, int32 Length)
	{
		do
		{
			uint8 Encoded = static_cast<uint8>(Length % 128);
			Length /= 128;
			if (Length > 0)
			{
				Encoded |= 0x80;
			}
			OutBytes.Add(Encoded);
		}
		while (Length > 0);
	}

	bool DecodeRemainingLength(const TArray<uint8>& Bytes, int32& OutLength, int32& OutBytesUsed)
	{
		int32 Multiplier = 1;
		OutLength = 0;
		OutBytesUsed = 0;

		for (int32 Index = 1; Index < Bytes.Num() && Index <= 4; ++Index)
		{
			const uint8 Encoded = Bytes[Index];
			OutLength += (Encoded & 127) * Multiplier;
			OutBytesUsed++;

			if ((Encoded & 128) == 0)
			{
				return true;
			}

			Multiplier *= 128;
		}

		return false;
	}

	FString BytesToUtf8String(const uint8* Data, int32 Len)
	{
		FUTF8ToTCHAR Converted(reinterpret_cast<const ANSICHAR*>(Data), Len);
		return FString(Converted.Length(), Converted.Get());
	}
}

APongMqttClient::APongMqttClient()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APongMqttClient::BeginPlay()
{
	Super::BeginPlay();
	ResolveGameManager();
	TryConnect();
}

void APongMqttClient::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GameManager == nullptr)
	{
		ResolveGameManager();
	}

	if (Socket == nullptr)
	{
		ReconnectTimer -= DeltaTime;
		if (ReconnectTimer <= 0.0f)
		{
			TryConnect();
		}
		return;
	}

	PollSocket();

	if (bMqttConnected)
	{
		PingTimer -= DeltaTime;
		if (PingTimer <= 0.0f)
		{
			SendPingPacket();
			PingTimer = 30.0f;
		}
	}
}

void APongMqttClient::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Disconnect();
	Super::EndPlay(EndPlayReason);
}

void APongMqttClient::TryConnect()
{
	Disconnect();

	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (SocketSubsystem == nullptr)
	{
		return;
	}

	TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
	bool bIsValidIp = false;
	Addr->SetIp(*BrokerHost, bIsValidIp);
	Addr->SetPort(BrokerPort);
	if (!bIsValidIp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Pong MQTT broker IP is invalid: %s"), *BrokerHost);
		ReconnectTimer = 3.0f;
		return;
	}

	Socket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("PongMqttSocket"), false);
	if (Socket == nullptr)
	{
		ReconnectTimer = 3.0f;
		return;
	}

	Socket->SetNonBlocking(false);
	Socket->SetNoDelay(true);
	if (!Socket->Connect(*Addr))
	{
		UE_LOG(LogTemp, Warning, TEXT("Pong MQTT failed to connect %s:%d"), *BrokerHost, BrokerPort);
		Disconnect();
		ReconnectTimer = 3.0f;
		return;
	}

	Socket->SetNonBlocking(true);
	SendConnectPacket();
	PingTimer = 30.0f;
	UE_LOG(LogTemp, Log, TEXT("Pong MQTT TCP connected to %s:%d"), *BrokerHost, BrokerPort);
}

void APongMqttClient::Disconnect()
{
	bMqttConnected = false;
	bSubscribed = false;
	ReceiveBuffer.Reset();

	if (Socket != nullptr)
	{
		Socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
		Socket = nullptr;
	}
}

void APongMqttClient::ResolveGameManager()
{
	for (TActorIterator<APongGameManager> It(GetWorld()); It; ++It)
	{
		GameManager = *It;
		break;
	}

	if (GameManager == nullptr && GetWorld() != nullptr)
	{
		GameManager = GetWorld()->SpawnActor<APongGameManager>();
		UE_LOG(LogTemp, Log, TEXT("Pong MQTT spawned PongGameManager automatically."));
	}

	if (GameManager != nullptr)
	{
		GameManager->OnMatchEnded.AddUniqueDynamic(this, &APongMqttClient::HandleMatchEnded);
		GameManager->OnBoardCountChanged.AddUniqueDynamic(this, &APongMqttClient::HandleBoardCountChanged);
	}
}

void APongMqttClient::SendConnectPacket()
{
	TArray<uint8> Body;
	AppendUtf8String(Body, TEXT("MQTT"));
	Body.Add(4);
	Body.Add(2);
	Body.Add(0);
	Body.Add(60);
	AppendUtf8String(Body, ClientId);

	TArray<uint8> Packet;
	Packet.Add(0x10);
	AppendRemainingLength(Packet, Body.Num());
	Packet.Append(Body);
	SendBytes(Packet);
}

void APongMqttClient::SendSubscribePacket()
{
	TArray<FString> Topics;
	Topics.Add(TEXT("GAME/#"));
	Topics.Add(TEXT("GAME/PLAYER/1/STATUS"));
	Topics.Add(TEXT("GAME/PLAYER/2/STATUS"));
	Topics.Add(TEXT("GAME/PLAYER/1/INPUT"));
	Topics.Add(TEXT("GAME/PLAYER/2/INPUT"));

	TArray<uint8> Body;
	Body.Add(0);
	Body.Add(1);
	for (const FString& Topic : Topics)
	{
		AppendUtf8String(Body, Topic);
		Body.Add(0);
	}

	TArray<uint8> Packet;
	Packet.Add(0x82);
	AppendRemainingLength(Packet, Body.Num());
	Packet.Append(Body);
	SendBytes(Packet);
	UE_LOG(LogTemp, Log, TEXT("Pong MQTT subscribed to STM32 topics."));
}

void APongMqttClient::SendPingPacket()
{
	TArray<uint8> Packet;
	Packet.Add(0xC0);
	Packet.Add(0);
	SendBytes(Packet);
}

void APongMqttClient::SendPublishPacket(const FString& Topic, const FString& Payload)
{
	TArray<uint8> Body;
	AppendUtf8String(Body, Topic);

	FTCHARToUTF8 Utf8Payload(*Payload);
	Body.Append(reinterpret_cast<const uint8*>(Utf8Payload.Get()), Utf8Payload.Length());

	TArray<uint8> Packet;
	Packet.Add(0x30);
	AppendRemainingLength(Packet, Body.Num());
	Packet.Append(Body);
	SendBytes(Packet);
}

void APongMqttClient::PollSocket()
{
	uint32 PendingSize = 0;
	while (Socket != nullptr && Socket->HasPendingData(PendingSize))
	{
		TArray<uint8> Chunk;
		Chunk.SetNumUninitialized(FMath::Min(static_cast<int32>(PendingSize), 4096));
		int32 BytesRead = 0;
		if (!Socket->Recv(Chunk.GetData(), Chunk.Num(), BytesRead) || BytesRead <= 0)
		{
			Disconnect();
			ReconnectTimer = 3.0f;
			return;
		}

		Chunk.SetNum(BytesRead);
		ReceiveBuffer.Append(Chunk);
	}

	while (ProcessNextPacket())
	{
	}
}

bool APongMqttClient::SendBytes(const TArray<uint8>& Bytes)
{
	if (Socket == nullptr || Bytes.Num() == 0)
	{
		return false;
	}

	int32 Sent = 0;
	const bool bOk = Socket->Send(Bytes.GetData(), Bytes.Num(), Sent);
	if (!bOk || Sent != Bytes.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Pong MQTT send failed."));
		Disconnect();
		ReconnectTimer = 3.0f;
		return false;
	}

	return true;
}

bool APongMqttClient::ProcessNextPacket()
{
	if (ReceiveBuffer.Num() < 2)
	{
		return false;
	}

	int32 RemainingLength = 0;
	int32 LengthBytes = 0;
	if (!DecodeRemainingLength(ReceiveBuffer, RemainingLength, LengthBytes))
	{
		return false;
	}

	const int32 HeaderLength = 1 + LengthBytes;
	const int32 TotalLength = HeaderLength + RemainingLength;
	if (ReceiveBuffer.Num() < TotalLength)
	{
		return false;
	}

	TArray<uint8> Body;
	Body.Append(ReceiveBuffer.GetData() + HeaderLength, RemainingLength);
	const uint8 PacketType = ReceiveBuffer[0] & 0xF0;
	ReceiveBuffer.RemoveAt(0, TotalLength);
	HandlePacket(PacketType, Body);
	return true;
}

void APongMqttClient::HandlePacket(uint8 PacketType, const TArray<uint8>& PacketBody)
{
	if (PacketType == 0x20)
	{
		bMqttConnected = PacketBody.Num() >= 2 && PacketBody[1] == 0;
		if (bMqttConnected && !bSubscribed)
		{
			SendSubscribePacket();
			bSubscribed = true;
		}
		UE_LOG(LogTemp, Log, TEXT("Pong MQTT CONNACK: %s"), bMqttConnected ? TEXT("OK") : TEXT("FAILED"));
	}
	else if (PacketType == 0x30)
	{
		HandlePublishPacket(PacketBody);
	}
	else if (PacketType == 0x90)
	{
		UE_LOG(LogTemp, Log, TEXT("Pong MQTT SUBACK received."));
	}
}

void APongMqttClient::HandlePublishPacket(const TArray<uint8>& PacketBody)
{
	if (PacketBody.Num() < 3)
	{
		return;
	}

	const int32 TopicLen = (static_cast<int32>(PacketBody[0]) << 8) | PacketBody[1];
	if (TopicLen <= 0 || 2 + TopicLen > PacketBody.Num())
	{
		return;
	}

	const FString Topic = BytesToUtf8String(PacketBody.GetData() + 2, TopicLen);
	const int32 PayloadOffset = 2 + TopicLen;
	const FString Payload = BytesToUtf8String(PacketBody.GetData() + PayloadOffset, PacketBody.Num() - PayloadOffset);

	UE_LOG(LogTemp, Log, TEXT("Pong MQTT message. Topic=%s Payload=%s"), *Topic, *Payload);

	if (Topic.EndsWith(TEXT("/STATUS")))
	{
		HandleStatusPayload(Topic, Payload);
	}
	else if (Topic.EndsWith(TEXT("/INPUT")))
	{
		HandleInputPayload(Topic, Payload);
	}
}

void APongMqttClient::HandleStatusPayload(const FString& Topic, const FString& Payload)
{
	if (GameManager == nullptr)
	{
		ResolveGameManager();
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Payload);
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return;
	}

	const int32 PlayerId = ExtractPlayerId(Topic, JsonObject);
	FString Status;
	JsonObject->TryGetStringField(TEXT("status"), Status);
	FString Ip = TEXT("Unknown");
	JsonObject->TryGetStringField(TEXT("ip"), Ip);

	if (GameManager != nullptr && (Status.IsEmpty() || Status.Equals(TEXT("online"), ESearchCase::IgnoreCase)))
	{
		GameManager->HandlePlayerStatusFromMqtt(PlayerId, Ip);
	}
}

void APongMqttClient::HandleInputPayload(const FString& Topic, const FString& Payload)
{
	if (GameManager == nullptr)
	{
		ResolveGameManager();
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Payload);
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return;
	}

	const int32 PlayerId = ExtractPlayerId(Topic, JsonObject);
	double Move = 0.0;
	JsonObject->TryGetNumberField(TEXT("move"), Move);

	if (GameManager != nullptr)
	{
		GameManager->HandlePlayerInputFromMqtt(PlayerId, static_cast<int32>(Move));
	}
}

int32 APongMqttClient::ExtractPlayerId(const FString& Topic, const TSharedPtr<FJsonObject>& JsonObject) const
{
	double Player = 0.0;
	if (JsonObject.IsValid() && JsonObject->TryGetNumberField(TEXT("player"), Player))
	{
		return FMath::Clamp(static_cast<int32>(Player), 1, 2);
	}

	if (Topic.Contains(TEXT("/PLAYER/2/")))
	{
		return 2;
	}

	return 1;
}

void APongMqttClient::HandleMatchEnded(int32 WinnerPlayerId, int32 LoserPlayerId, const FString& WinnerIp, const FString& LoserIp)
{
	if (!bMqttConnected)
	{
		return;
	}

	constexpr int32 ResultRepeatCount = 5;

	for (int32 RepeatIndex = 0; RepeatIndex < ResultRepeatCount; ++RepeatIndex)
	{
		SendPublishPacket(FString::Printf(TEXT("GAME/PLAYER/%d/RESULT"), WinnerPlayerId),
			FString::Printf(TEXT("{\"player\":%d,\"result\":\"win\"}"), WinnerPlayerId));
		SendPublishPacket(FString::Printf(TEXT("GAME/PLAYER/%d/RESULT"), LoserPlayerId),
			FString::Printf(TEXT("{\"player\":%d,\"result\":\"lose\"}"), LoserPlayerId));

		FPlatformProcess::Sleep(0.02f);
	}

	UE_LOG(LogTemp, Log, TEXT("Pong MQTT published match result x%d. Winner=%d Loser=%d"), ResultRepeatCount, WinnerPlayerId, LoserPlayerId);
}

void APongMqttClient::HandleBoardCountChanged(int32 ConnectedBoardCount)
{
	if (ConnectedBoardCount < 2)
	{
		bResetPublishedForCurrentReadyState = false;
		return;
	}

	if (bResetPublishedForCurrentReadyState)
	{
		return;
	}

	PublishResetToBoards();
	bResetPublishedForCurrentReadyState = true;
	UE_LOG(LogTemp, Log, TEXT("Pong MQTT reset scores at match-ready screen."));
}

void APongMqttClient::PublishResetToBoards()
{
	if (!bMqttConnected)
	{
		return;
	}

	constexpr int32 ResetRepeatCount = 5;

	for (int32 RepeatIndex = 0; RepeatIndex < ResetRepeatCount; ++RepeatIndex)
	{
		for (int32 PlayerId = 1; PlayerId <= 2; ++PlayerId)
		{
			SendPublishPacket(FString::Printf(TEXT("GAME/PLAYER/%d/RESULT"), PlayerId),
				FString::Printf(TEXT("{\"player\":%d,\"result\":\"reset\"}"), PlayerId));
		}

		FPlatformProcess::Sleep(0.03f);
	}

	UE_LOG(LogTemp, Log, TEXT("Pong MQTT published reset result to both STM32 boards x%d."), ResetRepeatCount);
}
