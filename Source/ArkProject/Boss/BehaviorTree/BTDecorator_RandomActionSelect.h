// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_RandomActionSelect.generated.h"

/**
 * 확률에 기반하여 행동을 랜덤하게 선택하는 데코레이터
 * 일반 추적과 특수 패턴(그라운드 공격) 중 하나를 선택하는 데 사용
 */
UCLASS()
class ARKPROJECT_API UBTDecorator_RandomActionSelect : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTDecorator_RandomActionSelect();
	
	// 조건 값 계산 함수 오버라이드
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
	// 데코레이터 초기화
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	
protected:
	// 특수 패턴 선택 확률 (0.0 ~ 1.0)
	UPROPERTY(EditAnywhere, Category = "Random", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SpecialPatternProbability = 0.3f;
	
	// 쿨다운 시간
	UPROPERTY(EditAnywhere, Category = "Random", meta = (ClampMin = "0.0"))
	float CooldownTime = 20.0f;
	
	// 마지막 특수 패턴 사용 시간을 저장할 블랙보드 키
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector LastSpecialPatternTimeKey;
	
	// 특수 패턴 사용 가능 여부를 저장할 블랙보드 키
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector CanUseSpecialPatternKey;
};
