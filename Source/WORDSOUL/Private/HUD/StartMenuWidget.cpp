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

	IPTextBox = Cast<UEditableText>(GetWidgetFromName(TEXT("IPInputBox")));
	if (!IPTextBox)
	{
		UE_LOG(LogTemp, Warning, TEXT("IPTextBoxCastFailed"));
	}

	ConfirmButton = Cast<UButton>(GetWidgetFromName(TEXT("btnConfirm")));
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &UStartMenuWidget::OnConfirmClicked);
	}

}

void UStartMenuWidget::OnStartButtonClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("WORDSOULMap"));
}

void UStartMenuWidget::OnTextChanged(const FText& Text)
{
	UWORDSOULGameInstance* GameInstance = Cast<UWORDSOULGameInstance>(GetWorld()->GetGameInstance());
	if (GameInstance)
	{
		GameInstance->IPAddress = Text.ToString();
	}
}

void UStartMenuWidget::OnConfirmClicked()
{
	UWORDSOULGameInstance* GameInstance = Cast<UWORDSOULGameInstance>(GetWorld()->GetGameInstance());
	if (GameInstance)
	{
		if (IPTextBox)
		{
			GameInstance->IPAddress = IPTextBox->GetText().ToString();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("IPTextBoxNone"));
		}
	}
}


