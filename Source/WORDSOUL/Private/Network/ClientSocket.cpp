// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/ClientSocket.h"

ClientSocket::ClientSocket()
{
}

ClientSocket::~ClientSocket()
{
	closesocket(sock);
	WSACleanup();
}

//=============================================================
bool ClientSocket::Init()
{
	UE_LOG(LogTemp, Warning, TEXT("Thread Initatlized"));

	return true;
}

uint32 ClientSocket::Run()
{

	return uint32();
}

void ClientSocket::Stop()
{

}

void ClientSocket::Exit()
{
	//?? 
}
//=============================================================

bool ClientSocket::InitSocket()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) return false;

	return true;
}

bool ClientSocket::ConnectToServer(const char* serverIP, int serverPort)
{
	struct sockaddr_in serveraddr;
	
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(serverPort);
	serveraddr.sin_addr.s_addr = inet_addr(serverIP);
	
	int retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 30.f, FColor::Green, "connect Failed");
		}
		return false;
	}
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 30.f, FColor::Green, "connect Success");
	}
	return true;
}
