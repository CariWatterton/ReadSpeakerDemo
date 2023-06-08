// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectBlackKatGameMode.h"
#include "ProjectBlackKatCharacter.h"
#include "UObject/ConstructorHelpers.h"

AProjectBlackKatGameMode::AProjectBlackKatGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
