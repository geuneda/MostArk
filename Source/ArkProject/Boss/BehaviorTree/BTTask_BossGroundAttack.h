// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BossBTTaskBase.h"
#include "BTTask_BossGroundAttack.generated.h"

/**
 * 보스의 그라운드 공격 패턴을 실행하는 태스크
 */
UCLASS()
class ARKPROJECT_API UBTTask_BossGroundAttack : public UBossBTTaskBase
{
	GENERATED_BODY()
	
public:
	UBTTask_BossGroundAttack();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
private:
	// 그라운드 공격 지속 시간
	UPROPERTY(EditAnywhere, Category = "Attack", meta = (AllowPrivateAccess = "true"))
	float GroundAttackDuration = 7.0f;
	
	// 현재 경과 시간
	float CurrentTime = 0.0f;
	
	// 공격이 시작되었는지 여부
	bool bAttackStarted = false;
};
