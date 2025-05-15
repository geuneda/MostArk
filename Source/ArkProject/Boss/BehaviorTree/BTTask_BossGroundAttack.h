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
	
	// VFX 정리 함수
	void CleanupVFX();
	
private:
	// 그라운드 공격 지속 시간
	UPROPERTY(EditAnywhere, Category = "Attack", meta = (AllowPrivateAccess = "true"))
	float GroundAttackDuration = 7.0f;
	
	// 플레이어 추적 시간 (공격 종료 전에 멈추기 위한 시간)
	UPROPERTY(EditAnywhere, Category = "Attack", meta = (AllowPrivateAccess = "true"))
	float StopFollowingTime = 5.0f;
	
	// 플레이어 추적 업데이트 간격
	UPROPERTY(EditAnywhere, Category = "Attack", meta = (AllowPrivateAccess = "true"))
	float UpdateInterval = 0.5f;
	
	// 목적지에 도달했다고 판단하는 거리
	UPROPERTY(EditAnywhere, Category = "Attack", meta = (AllowPrivateAccess = "true"))
	float AcceptanceRadius = 200.0f;
	
	// 현재 경과 시간
	float CurrentTime = 0.0f;
	
	// 마지막 위치 업데이트 시간
	float LastUpdateTime = 0.0f;
	
	// 공격이 시작되었는지 여부
	bool bAttackStarted = false;
	
	// 플레이어 추적 중인지 여부
	bool bIsFollowingPlayer = false;
	
	// 시작 VFX 컴포넌트 참조
	class UNiagaraComponent* GroundAttackVFXComponent = nullptr;
	
	// 종료 VFX 컴포넌트 참조
	class UNiagaraComponent* GroundAttackFinishVFXComponent = nullptr;
	
	// VFX 생성 위치
	FVector VFXSpawnLocation = FVector::ZeroVector;
};
