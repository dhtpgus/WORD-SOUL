// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "WORDSOULGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class WORDSOUL_API UWORDSOULGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FString IPAddress;
};
