// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterType.h"
#include "WORDSOULAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class WORDSOUL_API UWORDSOULAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	UPROPERTY(BlueprintReadOnly)
	class AWORDSOULCharacter* WORDSOULCharacter;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	class UCharacterMovementComponent* WORDSOULCharacterMovement;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool IsFalling;

	UPROPERTY(BlueprintReadOnly, Category = "Movement | Character State")
	ECharacterState CharacterState;

	UPROPERTY(BlueprintReadOnly, Category = "Movement | Action State")
	EActionState ActionState;
};
