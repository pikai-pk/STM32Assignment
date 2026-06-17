#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PongMqttClient.generated.h"

class APongGameManager;
class FSocket;

UCLASS()
class PONGMQTT_API APongMqttClient : public AActor
{
	GENERATED_BODY()

public:
	APongMqttClient();

	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	virtual void BeginPlay() override;

private:
	void TryConnect();
	void Disconnect();
	void ResolveGameManager();
	void SendConnectPacket();
	void SendSubscribePacket();
	void SendPingPacket();
	void SendPublishPacket(const FString& Topic, const FString& Payload);
	void PollSocket();
	bool SendBytes(const TArray<uint8>& Bytes);
	bool ProcessNextPacket();
	void HandlePacket(uint8 PacketType, const TArray<uint8>& PacketBody);
	void HandlePublishPacket(const TArray<uint8>& PacketBody);
	void HandleStatusPayload(const FString& Topic, const FString& Payload);
	void HandleInputPayload(const FString& Topic, const FString& Payload);
	int32 ExtractPlayerId(const FString& Topic, const TSharedPtr<class FJsonObject>& JsonObject) const;
	void PublishResetToBoards();

	UFUNCTION()
	void HandleMatchEnded(int32 WinnerPlayerId, int32 LoserPlayerId, const FString& WinnerIp, const FString& LoserIp);

	UFUNCTION()
	void HandleBoardCountChanged(int32 ConnectedBoardCount);

	UPROPERTY(EditAnywhere, Category = "Pong|MQTT")
	FString BrokerHost = TEXT("127.0.0.1");

	UPROPERTY(EditAnywhere, Category = "Pong|MQTT")
	int32 BrokerPort = 1883;

	UPROPERTY(EditAnywhere, Category = "Pong|MQTT")
	FString ClientId = TEXT("UE_PONG_CLIENT");

	UPROPERTY()
	APongGameManager* GameManager = nullptr;

	FSocket* Socket = nullptr;
	TArray<uint8> ReceiveBuffer;
	float ReconnectTimer = 0.0f;
	float PingTimer = 0.0f;
	bool bMqttConnected = false;
	bool bSubscribed = false;
	bool bResetPublishedForCurrentReadyState = false;
};
