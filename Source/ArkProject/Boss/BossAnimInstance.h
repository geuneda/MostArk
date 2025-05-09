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
	
	void InitializeBoss();
	
	virtual void NativeBeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class ABoss* Boss;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= Animation)
	bool bIsRun;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= Animation)
	bool bIsAttack;

	// 왼손 공격
	void PlayLeftAttackMontage();
	UFUNCTION()
	void AnimNotify_LeftAttackStart();
	UFUNCTION()
	void AnimNotify_LeftAttackEnd();

	// 오른손 공격
	void PlayRightAttackMontage();
	UFUNCTION()
	void AnimNotify_RightAttackStart();
	UFUNCTION()
	void AnimNotify_RightAttackEnd();

	// 꼬리 공격
	void PlayTailAttackMontage();
	UFUNCTION()
	void AnimNotify_TailAttackStart();
	UFUNCTION()
	void AnimNotify_TailAttackEnd();

	// 그라운드 패턴
	void PlayGroundAttackMontage();
	UFUNCTION()
	void AnimNotify_Hide();
	UFUNCTION()
	void AnimNotify_Show();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= Animation)
	class UAnimMontage* GroundAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= Animation)
	class UAnimMontage* LeftAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= Animation)
	class UAnimMontage* RightAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= Animation)
	class UAnimMontage* TailAttackMontage;
};
