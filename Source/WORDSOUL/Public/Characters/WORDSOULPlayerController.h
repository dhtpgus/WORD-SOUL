// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NetWork/ClientSocket.h"
#include "Characters/WORDSOULCharacter.h"
#include "NetWork/Packet.h"
#include "WORDSOULPlayerController.generated.h"

/**
 * 
 */

UCLASS()
class WORDSOUL_API AWORDSOULPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AWORDSOULPlayerController();
	virtual ~AWORDSOULPlayerController();

	virtual void Tick(float DeltaTime) override;
	
	void RecvCharacterInfo(SCPosition* CharacterInfo);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(BlueprintReadWrite)
	AWORDSOULCharacter* OtherCharacter;
	
private:
	ClientSocket* Socket;
	bool bIsConnected;
	uint8 id;
	SCPosition* OtherCharacterInfo;
	
	void UpdatePlayerInfo(const SCPosition& CharacterInfo);

};
