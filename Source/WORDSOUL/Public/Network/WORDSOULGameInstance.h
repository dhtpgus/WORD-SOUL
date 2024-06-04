// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Network/ClientSocket.h"
#include "WORDSOULGameInstance.generated.h"

class AWORDSOULPlayerController;

UCLASS()
class WORDSOUL_API UWORDSOULGameInstance : public UGameInstance
{
	GENERATED_BODY()
	UWORDSOULGameInstance();
	
public:
	UPROPERTY()
	FString IPAddress;

	void InitNetwork();
	bool IsConnected() const { return bIsConnected; }
	void EndNetwork();

	ClientSocket* Socket;
	bool bIsConnected;

};
