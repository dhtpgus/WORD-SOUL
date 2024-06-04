// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/WORDSOULGameInstance.h"
#include "Characters/WORDSOULPlayerController.h"

UWORDSOULGameInstance::UWORDSOULGameInstance()
{

}

void UWORDSOULGameInstance::InitNetwork()
{
	const char* ServerIPChars = TCHAR_TO_ANSI(*IPAddress);

	Socket = ClientSocket::GetInstance();
	Socket->InitSocket();
	bIsConnected = Socket->ConnectToServer(ServerIPChars, SERVER_PORT);
	if (bIsConnected)
	{
		UE_LOG(LogTemp, Warning, TEXT("SERVER CONNECT SUCCESS"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SERVER CONNECT FAILED"));
	}
}

void UWORDSOULGameInstance::EndNetwork()
{
	if (bIsConnected and Socket)
	{
		Socket->LeaveParty();
		Socket->EndRecvThread();
	}
}


