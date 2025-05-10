// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BossBTTaskBase.h"
#include "BTTask_DetectPlayer.generated.h"

/**
 * 플레이어를 감지하는 태스크
 */
UCLASS()
class ARKPROJECT_API UBTTask_DetectPlayer : public UBossBTTaskBase
{
	GENERATED_BODY()
	
public:
	UBTTask_DetectPlayer();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
private:
	// 플레이어 감지 범위
	UPROPERTY(EditAnywhere, Category = "Detection", meta = (AllowPrivateAccess = "true"))
	float DetectionRadius = 1500.0f;
};
