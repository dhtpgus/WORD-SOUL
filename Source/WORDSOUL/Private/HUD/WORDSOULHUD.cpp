// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/WORDSOULHUD.h"
#include "HUD/WORDSOULOverlay.h"

void AWORDSOULHUD::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController and WORDSOULOverlayClass)
		{
			WORDSOULOverlay = CreateWidget<UWORDSOULOverlay>(PlayerController, WORDSOULOverlayClass);
			WORDSOULOverlay->AddToViewport();
		}
	}
}
