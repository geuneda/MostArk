// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ArkProjectCharacter.generated.h"

UCLASS(Blueprintable)
class AArkProjectCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AArkProjectCharacter();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;


};

