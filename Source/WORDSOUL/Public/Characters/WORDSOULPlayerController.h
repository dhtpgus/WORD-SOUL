// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NetWork/ClientSocket.h"
#include "Characters/WORDSOULCharacter.h"
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
	
	void RecvCharacterInfo(SCCharacterInfo* CharacterInfo);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(BlueprintReadWrite)
	AWORDSOULCharacter* OtherCharacter;
	
private:
	ClientSocket* Socket;
	bool bIsConnected;
	uint8 id;
	SCCharacterInfo* OtherCharacterInfo;

	void UpdatePlayerInfo(const SCCharacterInfo& CharacterInfo);

};
