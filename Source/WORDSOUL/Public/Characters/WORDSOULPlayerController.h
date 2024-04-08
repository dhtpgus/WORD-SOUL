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


protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, Category = "SpawnCharacter")
	TSubclassOf<class ACharacter> SpawnCharacter;
	
private:
	ClientSocket* Socket;
	bool bIsConnected;
	uint8 id;

};
