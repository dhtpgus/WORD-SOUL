// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/WORDSOULOverlay.h"
#include "Components/ProgressBar.h"

void UWORDSOULOverlay::SetHealthBarPercent(float Percent)
{
	if (HealthBar)
	{
		HealthBar->SetPercent(Percent);
	}
}

void UWORDSOULOverlay::SetStaminaBarPercent(float Percent)
{
	if (StaminaBar)
	{
		StaminaBar->SetPercent(Percent);
	}
}
