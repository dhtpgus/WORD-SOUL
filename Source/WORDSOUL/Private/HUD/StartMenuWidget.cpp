// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/StartMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Button.h"

void UStartMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	StartButton = Cast<UButton>(GetWidgetFromName(TEXT("StartButton")));
	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &UStartMenuWidget::OnStartButtonClicked);
	}
}

void UStartMenuWidget::OnStartButtonClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("WORDSOULMap"));
}


