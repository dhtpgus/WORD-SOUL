// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "WORDSOULHUD.generated.h"

class UWORDSOULOverlay;

UCLASS()
class WORDSOUL_API AWORDSOULHUD : public AHUD
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY(EditDefaultsOnly, Category = WORDSOUL)
	TSubclassOf<UWORDSOULOverlay> WORDSOULOverlayClass;

	UPROPERTY()
	UWORDSOULOverlay* WORDSOULOverlay;
public:
	FORCEINLINE UWORDSOULOverlay* GetWORDSOULOverlay() const { return WORDSOULOverlay; };
};
