// Copyright Epic Games, Inc. All Rights Reserved.

#include "WORDSOULGameMode.h"
#include "WORDSOULCharacter.h"
#include "UObject/ConstructorHelpers.h"

AWORDSOULGameMode::AWORDSOULGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
