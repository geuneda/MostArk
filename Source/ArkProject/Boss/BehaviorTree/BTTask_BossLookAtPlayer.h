// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BossBTTaskBase.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_BossLookAtPlayer.generated.h"

/**
 * 보스가 플레이어 방향(랜덤)을 바라보는 태스크
 * 자연스러움을 위해 정확히 플레이어를 바라보지 않고 약간의 랜덤성을 추가함
 */
UCLASS()
class ARKPROJECT_API UBTTask_BossLookAtPlayer : public UBossBTTaskBase
{
	GENERATED_BODY()
	
public:
	UBTTask_BossLookAtPlayer();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
protected:
	/** 회전 보간 속도 (값이 클수록 빠르게 회전) */
	UPROPERTY(EditAnywhere, Category = "Look Parameters", meta = (ClampMin = "0.1", ClampMax = "20.0"))
	float RotationSpeed;
	
	/** 랜덤 오프셋 최대 각도 (도) */
	UPROPERTY(EditAnywhere, Category = "Look Parameters", meta = (ClampMin = "0.0", ClampMax = "45.0"))
	float MaxRandomAngleOffset;
	
	/** 랜덤 위치 오프셋 최대 거리 */
	UPROPERTY(EditAnywhere, Category = "Look Parameters", meta = (ClampMin = "0.0", ClampMax = "300.0"))
	float MaxRandomPositionOffset;
	
	/** 태스크 완료 전 최소 회전 일치도 (0-1) (정확도 개념) */
	UPROPERTY(EditAnywhere, Category = "Look Parameters", meta = (ClampMin = "0.9", ClampMax = "0.99"))
	float MinRotationMatchThreshold;
};
