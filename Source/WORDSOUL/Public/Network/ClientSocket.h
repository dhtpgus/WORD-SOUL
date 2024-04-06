// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#pragma comment(lib, "ws2_32.lib")
#include <WS2tcpip.h>

#define SERVER_PORT 9000
#define SERVER_IP "127.0.0.1"
#define BUFSIZE 1024

#pragma pack(push, 1)
struct CSCharacterLocation
{
	uint8 length;
	uint8 packetNum;
	float x;
	float y;
	float z;
};

struct SCCharacterInfo
{
	uint8 length;
	uint8 packetNum;
	uint16 id;
	float x;
	float y;
	float z;
};

struct party
{
	uint8 length;
	uint8 packetNum;
	uint16 id;
};
#pragma pack(pop)
/**
 * 
 */
class WORDSOUL_API ClientSocket : FRunnable
{
public:
	ClientSocket();
	~ClientSocket();

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

	bool InitSocket();
	bool ConnectToServer(const char* serverIP, int serverPort);
	void SendCharacterLocation(const FVector& CharacterLocation);
	void Party();
	TUniquePtr<SCCharacterInfo> RecvCharacterInfo();

private:
	//=============================================================
	FRunnableThread* Thread;
	//=============================================================

	SOCKET sock;
	char buffer[BUFSIZE];
	
};
