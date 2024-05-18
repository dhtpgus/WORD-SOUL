// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WORDSOULOverlay.generated.h"

class UProgressBar;

UCLASS()
class WORDSOUL_API UWORDSOULOverlay : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetHealthBarPercent(float Percent);
	void SetStaminaBarPercent(float Percent);

private:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* StaminaBar;
};
