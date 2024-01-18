// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WORDSOULCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class WORDSOUL_API AWORDSOULCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AWORDSOULCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;
	void MoveForward(float value);
	void Turn(float value);
	void LookUp(float value);
	void MoveRight(float value);

private:
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

};
