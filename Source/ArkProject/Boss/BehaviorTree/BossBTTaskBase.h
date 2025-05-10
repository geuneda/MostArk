// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BossBTTaskBase.generated.h"

/**
 * 보스 행동 트리 태스크의 기본 클래스
 */
UCLASS(Abstract)
class ARKPROJECT_API UBossBTTaskBase : public UBTTaskNode
{
	GENERATED_BODY()
	
protected:
	// 보스 캐릭터 참조 가져오기
	class ABoss* GetBossCharacter(UBehaviorTreeComponent* OwnerComp) const;
	
	// 보스 컨트롤러 참조 가져오기
	class AAIController* GetBossController(UBehaviorTreeComponent* OwnerComp) const;
	
	// 플레이어 캐릭터 참조 가져오기
	class ACharacter* GetPlayerCharacter(UBehaviorTreeComponent* OwnerComp) const;
};
