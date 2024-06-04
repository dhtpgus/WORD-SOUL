// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "NetWork/Packet.h"
#pragma comment(lib, "ws2_32.lib")
#include <WS2tcpip.h>

#define SERVER_PORT 9000
#define SERVER_IP "127.0.0.1"
#define BUFSIZE 1024
#define PACKET_TYPE_OFFSET 1


/**
 * 
 */
class AWORDSOULPlayerController;

class WORDSOUL_API ClientSocket : FRunnable
{
public:
	ClientSocket();
	~ClientSocket();

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

	static ClientSocket* GetInstance()
	{
		static ClientSocket Ins;
		return &Ins;
	}

	bool InitSocket();
	bool ConnectToServer(const char* serverIP, int serverPort);
	bool StartRecvThread();
	void EndRecvThread();
	void SendCharacterInfo(const FVector& location, float groundSpeed, char flag, float rotation);
	void Party();
	void LeaveParty();
	void SetPlayerController(AWORDSOULPlayerController* playerController);

private:
	//=============================================================
	FRunnableThread* Thread;
	bool bStop;
	//=============================================================

	SOCKET sock;
	char recvBuffer[BUFSIZE];

	AWORDSOULPlayerController* PlayerController;
};
