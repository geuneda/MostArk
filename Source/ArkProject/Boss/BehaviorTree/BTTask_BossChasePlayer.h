// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BossBTTaskBase.h"
#include "BTTask_BossChasePlayer.generated.h"

/**
 * 보스가 플레이어를 추적하는 태스크
 */
UCLASS()
class ARKPROJECT_API UBTTask_BossChasePlayer : public UBossBTTaskBase
{
	GENERATED_BODY()
	
public:
	UBTTask_BossChasePlayer();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
private:
	// 목적지에 도달했다고 판단하는 거리
	UPROPERTY(EditAnywhere, Category = "Chase", meta = (AllowPrivateAccess = "true"))
	float AcceptanceRadius = 200.0f;
	
	// 플레이어 위치 업데이트 간격
	UPROPERTY(EditAnywhere, Category = "Chase", meta = (AllowPrivateAccess = "true"))
	float UpdateInterval = 0.5f;
	
	// 마지막 위치 업데이트 시간
	float LastUpdateTime = 0.0f;
};
