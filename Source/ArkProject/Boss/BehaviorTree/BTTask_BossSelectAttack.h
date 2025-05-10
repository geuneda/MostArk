// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BossBTTaskBase.h"
#include "BTTask_BossSelectAttack.generated.h"

/**
 * 보스가 플레이어와의 각도에 따라 공격 방식을 선택하는 태스크
 */
UCLASS()
class ARKPROJECT_API UBTTask_BossSelectAttack : public UBossBTTaskBase
{
	GENERATED_BODY()
	
public:
	UBTTask_BossSelectAttack();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
private:
	// 정면 공격 각도 범위 (-45도 ~ 45도)
	UPROPERTY(EditAnywhere, Category = "Attack", meta = (AllowPrivateAccess = "true"))
	float FrontAttackAngleThreshold = 45.0f;
	
	// 측면 이동 공격 각도 범위 (46도 ~ 90도, -46도 ~ -90도)
	UPROPERTY(EditAnywhere, Category = "Attack", meta = (AllowPrivateAccess = "true"))
	float SideAttackAngleThreshold = 90.0f;
	
	// 정면 공격 중 왼손 공격 각도 범위 (-45도 ~ -15도)
	UPROPERTY(EditAnywhere, Category = "Attack", meta = (AllowPrivateAccess = "true"))
	float LeftAttackAngleThreshold = -15.0f;
	
	// 정면 공격 중 꼬리 공격 각도 범위 (-15도 ~ 15도)
	UPROPERTY(EditAnywhere, Category = "Attack", meta = (AllowPrivateAccess = "true"))
	float TailAttackAngleThreshold = 15.0f;
	
	// 정면 공격 중 오른손 공격 각도 범위 (15도 ~ 45도)
	UPROPERTY(EditAnywhere, Category = "Attack", meta = (AllowPrivateAccess = "true"))
	float RightAttackAngleThreshold = 45.0f;
	
	// 플레이어와의 각도 계산
	float CalculateAngleToPlayer(ABoss* Boss, ACharacter* Player);
};
