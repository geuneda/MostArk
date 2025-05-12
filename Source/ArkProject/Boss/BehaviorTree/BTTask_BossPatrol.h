// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BossBTTaskBase.h"
#include "BTTask_BossPatrol.generated.h"

/**
 * 보스가 내비게이션 메시를 기반으로 정찰하는 태스크
 */
UCLASS()
class ARKPROJECT_API UBTTask_BossPatrol : public UBossBTTaskBase
{
	GENERATED_BODY()
	
public:
	UBTTask_BossPatrol();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

private:
	// 정찰 범위 (보스 주변 반경)
	UPROPERTY(EditAnywhere, Category = "Patrol", meta = (AllowPrivateAccess = "true"))
	float PatrolRadius = 1000.0f;
	
	// 다음 정찰 지점을 찾을 때까지 대기 시간
	UPROPERTY(EditAnywhere, Category = "Patrol", meta = (AllowPrivateAccess = "true"))
	float WaitTime = 2.0f;
	
	// 정찰 지점에 도달했다고 판단하는 거리
	UPROPERTY(EditAnywhere, Category = "Patrol", meta = (AllowPrivateAccess = "true"))
	float AcceptanceRadius = 50.0f;
	
	// 현재 대기 시간
	float CurrentWaitTime = 5.0f;
	
	// 목적지에 도달했는지 여부
	bool bHasReachedDestination = false;
	
	// 새로운 정찰 지점 찾기
	bool FindNewPatrolPoint(UBehaviorTreeComponent& OwnerComp);
};
