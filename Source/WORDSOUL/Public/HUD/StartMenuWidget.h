// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableText.h"
#include "Network/WORDSOULGameInstance.h"
#include "StartMenuWidget.generated.h"

class UButton;

UCLASS()
class WORDSOUL_API UStartMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* StartButton;

	UPROPERTY()
	UEditableText* IPTextBox;

	UPROPERTY()
	UButton* ConfirmButton;

	
private:
	UFUNCTION(BlueprintCallable)
	void OnStartButtonClicked();

	UFUNCTION(BlueprintCallable)
	void OnTextChanged(const FText& Text);

	UFUNCTION()
	void OnConfirmClicked();
	
	
};
