// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class ARKPROJECT_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float WalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool Attack1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool Attack2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool Attack3;
};
