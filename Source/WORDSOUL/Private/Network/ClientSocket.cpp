// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/ClientSocket.h"

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
				case 2:
					if(totalBytes >= sizeof(SCCharacterInfo))
					{
						SCCharacterInfo* characterInfo = reinterpret_cast<SCCharacterInfo*>(bufferPtr);
						UE_LOG(LogTemp, Warning, TEXT("messageLength : %d"), characterInfo->length);
						UE_LOG(LogTemp, Warning, TEXT("packet num : %d"), characterInfo->packetNum);
						UE_LOG(LogTemp, Warning, TEXT("Other Character id : %d"), characterInfo->id);
						UE_LOG(LogTemp, Warning, TEXT("Character x y z : %f  %f  %f"), characterInfo->x, characterInfo->y, characterInfo->z);
					}
					
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

void ClientSocket::SetPlayerController(AWORDSOULPlayerController* playerController)
{
	if (playerController)
	{
		PlayerController = playerController;
	}
}


// 서버에서 클라르 보내주는 패킷
// 파티 입장패킷 보내고 -> 파티 제대로 입장햇는지 패킷 서버-클라(최초 한번) -> 다른사람의 좌표값 패킷(2번)

