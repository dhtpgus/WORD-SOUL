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

void ClientSocket::SendCharacterLocation(const FVector& CharacterLocation)
{
	CSCharacterLocation CharacterLoc;
	CharacterLoc.length = 12;
	CharacterLoc.packetNum = 129;
	CharacterLoc.x = CharacterLocation.X;
	CharacterLoc.y = CharacterLocation.Y;
	CharacterLoc.z = CharacterLocation.Z;

	send(sock, (char*)&CharacterLoc, sizeof(CSCharacterLocation), 0);
}

void ClientSocket::Party()
{
	party party0;
	party0.id = 0;
	party0.length = sizeof(party) - 2;
	party0.packetNum = 128;
	send(sock, (char*)&party0, sizeof(party), 0);
}



TUniquePtr<SCCharacterInfo> ClientSocket::RecvCharacterInfo()
{
	TUniquePtr<SCCharacterInfo> CharacterInfo = MakeUnique<SCCharacterInfo>();
	int recvLen = recv(sock, (char*)CharacterInfo.Get(), sizeof(SCCharacterInfo), 0);
	
	if (recvLen != sizeof(SCCharacterInfo))
	{
		UE_LOG(LogTemp, Warning, TEXT("Return nullptr : recvLen = %d"), recvLen);
		return nullptr;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("recvLen = %d"), recvLen);
	}
		
	return CharacterInfo;
}


// 서버에서 클라르 보내주는 패킷
// 파티 입장패킷 보내고 -> 파티 제대로 입장햇는지 패킷 서버-클라(최초 한번) -> 다른사람의 좌표값 패킷(2번)

