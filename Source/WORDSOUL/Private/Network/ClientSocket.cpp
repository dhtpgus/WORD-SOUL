// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/ClientSocket.h"
#include "Network/Packet.h"
#include "Characters/WORDSOULPlayerController.h"

ClientSocket::ClientSocket()
{

}

ClientSocket::~ClientSocket()
{
	if (Thread)
	{
		delete Thread;
		Thread = nullptr;
	}

	closesocket(sock);
	WSACleanup();
}

//=============================================================
bool ClientSocket::Init()
{
	UE_LOG(LogTemp, Warning, TEXT("Thread Initatlized"));
	bStop = false;

	return true;
}

uint32 ClientSocket::Run()
{
	FPlatformProcess::Sleep(0.03);

	int bytesReceived;
	int remainBytes = 0;
	char* bufferPtr = recvBuffer;

	while (!bStop and (bytesReceived = recv(sock, recvBuffer + remainBytes, sizeof(recvBuffer) - remainBytes, 0)) > 0)
	{
		if (bytesReceived == SOCKET_ERROR) return 0;

		int totalBytes = bytesReceived + remainBytes;
		bufferPtr = recvBuffer;

		while (totalBytes > 1)
		{
			uint8 messageLength = *bufferPtr;
			uint8 packetType = static_cast<uint8>(*(bufferPtr + PACKET_TYPE_OFFSET));
			int packetSize = messageLength + 2;
			
			if (totalBytes >= packetSize)
			{
				switch (packetType)
				{
				case (uint8)EPacketType::SCNewEntity:
					break;
				case (uint8)EPacketType::SCPosition:
					if (totalBytes >= sizeof(SCPosition))
					{
						SCPosition characterInfo;
						if (memcpy(&characterInfo, bufferPtr, sizeof(SCPosition)))
						{
							if (characterInfo.id == 0xFFFF) // OtherPlayer
							{
								PlayerController->RecvCharacterInfo(characterInfo);
								//UE_LOG(LogTemp, Warning, TEXT("Character x y z : %f  %f  %f"), characterInfo->x, characterInfo->y, characterInfo->z);
							}
						}
					}	
					break;
				case (uint8)EPacketType::SCRemoveEntity:
					break;
				case (uint8)EPacketType::SCResult:
					break;
				case (uint8)EPacketType::SCCheckConnection:
					//UE_LOG(LogTemp, Warning, TEXT("TEST MESSAGE"));
					break;
				case (uint8)EPacketType::SCModifyHp:
					break;
				default:
					break;
				}
				bufferPtr += packetSize;
				totalBytes -= packetSize;
			}
			else
			{
				break;
			}
		}
		if (totalBytes > 0)
		{
			memmove(recvBuffer, bufferPtr, totalBytes);
		}

		remainBytes = totalBytes;
	}

	return uint32();
}

void ClientSocket::Stop()
{
	bStop = true;
}

void ClientSocket::Exit()
{
	
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

bool ClientSocket::StartRecvThread()
{
	if (Thread != nullptr) return false;
	Thread = FRunnableThread::Create(this, TEXT("ClientSocket"));
	return (Thread != nullptr);
}

void ClientSocket::EndRecvThread()
{
	if (Thread)
	{
		Stop();
		Thread->WaitForCompletion();
		Thread->Kill();
		delete Thread;
		Thread = nullptr;
	}
}

void ClientSocket::SendCharacterInfo(const FVector& location, float groundSpeed, char flag, float rotation)
{
	CSPosition CharacterInfo;
	CharacterInfo.length = sizeof(CSPosition) - sizeof(PacketBase);
	CharacterInfo.packetType = EPacketType::CSPosition;
	CharacterInfo.x = location.X;
	CharacterInfo.y = location.Y;
	CharacterInfo.z = location.Z;
	CharacterInfo.v = groundSpeed;
	CharacterInfo.r = rotation;
	CharacterInfo.flag = flag; 

	send(sock, (char*)&CharacterInfo, sizeof(CSPosition), 0);
}

void ClientSocket::Party()
{
	CSJoinParty partyPacket;
	partyPacket.id = 0;
	partyPacket.length = sizeof(CSJoinParty) - sizeof(PacketBase);
	partyPacket.packetType = EPacketType::CSJoinParty;
	send(sock, (char*)&partyPacket, sizeof(CSJoinParty), 0);
}

void ClientSocket::SetPlayerController(AWORDSOULPlayerController* playerController)
{
	if (playerController)
	{
		PlayerController = playerController;
	}
}

