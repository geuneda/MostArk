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

	// 왼쪽 무빙 공격
	void PlayLeftMoveAttackMontage() const;

	// 왼손 공격
	void PlayLeftAttackMontage() const;
	UFUNCTION()
	void AnimNotify_LeftAttackStart();
	UFUNCTION()
	void AnimNotify_LeftAttackEnd();

	// 오른쪽 무빙 공격
	void PlayRightMoveAttackMontage() const;

	// 오른손 공격
	void PlayRightAttackMontage() const;
	UFUNCTION()
	void AnimNotify_RightAttackStart();
	UFUNCTION()
	void AnimNotify_RightAttackEnd();

	// 꼬리 공격
	void PlayTailAttackMontage() const;
	UFUNCTION()
	void AnimNotify_TailAttackStart();
	UFUNCTION()
	void AnimNotify_TailAttackEnd();

	// 그라운드 패턴
	UFUNCTION()
	void PlayGroundAttackMontage() const;
	UFUNCTION()
	void StopGroundAttackMontage() const;
	UFUNCTION()
	void AnimNotify_Hide();
	UFUNCTION()
	void AnimNotify_Show();
	UFUNCTION()
	void AnimNotify_GroundAttackStart();
	UFUNCTION()
	void AnimNotify_GroundAttackEnd();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= Animation)
	class UAnimMontage* GroundAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= Animation)
	class UAnimMontage* LeftAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= Animation)
	class UAnimMontage* RightAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= Animation)
	class UAnimMontage* TailAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= Animation)
	class UAnimMontage* LeftMoveAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= Animation)
	class UAnimMontage* RightMoveAttackMontage;
};
