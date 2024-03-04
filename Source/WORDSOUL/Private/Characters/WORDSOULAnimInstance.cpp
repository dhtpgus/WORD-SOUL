// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/WORDSOULAnimInstance.h"
#include "Characters/WORDSOULCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UWORDSOULAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	WORDSOULCharacter = Cast<AWORDSOULCharacter>(TryGetPawnOwner());
	if (WORDSOULCharacter)
	{
		WORDSOULCharacterMovement = WORDSOULCharacter->GetCharacterMovement();
	}
}

void UWORDSOULAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (WORDSOULCharacterMovement)
	{
		GroundSpeed = UKismetMathLibrary::VSizeXY(WORDSOULCharacterMovement->Velocity);
		IsFalling = WORDSOULCharacterMovement->IsFalling();
		CharacterState = WORDSOULCharacter->GetCharacterState();
		ActionState = WORDSOULCharacter->GetActionState();
	}
}
