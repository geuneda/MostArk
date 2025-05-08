// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BossAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class ARKPROJECT_API UBossAnimInstance : public UAnimInstance
{
	GENERATED_BODY()


public:
	virtual void NativeBeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class ABoss* Boss;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= Animation)
	bool bIsRun;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= Animation)
	bool bIsAttack;

	UFUNCTION()
	void AnimNotify_LeftAttackStart();
	UFUNCTION()
	void AnimNotify_LeftAttackEnd();
	
	UFUNCTION()
	void AnimNotify_RightAttackStart();
	UFUNCTION()
	void AnimNotify_RightAttackEnd();
	
	UFUNCTION()
	void AnimNotify_TailAttackStart();
	UFUNCTION()
	void AnimNotify_TailAttackEnd();
};
