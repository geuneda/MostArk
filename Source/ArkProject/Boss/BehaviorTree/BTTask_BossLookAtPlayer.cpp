// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_BossLookAtPlayer.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "../Boss.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_BossLookAtPlayer::UBTTask_BossLookAtPlayer()
{
	bNotifyTick = true;
	// 노드 이름 설정
	NodeName = TEXT("Look At Player");
	
	// 기본값 설정
	RotationSpeed = 2.0f;
	MaxRandomAngleOffset = 10.0f;
	MaxRandomPositionOffset = 100.0f;
	MinRotationMatchThreshold = 0.95f;
}

// TickTask에서 사용할 메모리 구조체 정의
struct FBossLookAtPlayerMemory
{
	FRotator TargetRotation;
	bool bInitialized;
};

EBTNodeResult::Type UBTTask_BossLookAtPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 메모리 초기화
	FBossLookAtPlayerMemory* Memory = reinterpret_cast<FBossLookAtPlayerMemory*>(NodeMemory);
	Memory->bInitialized = false;
	
	// 보스 컨트롤러와 캐릭터 가져오기
	AAIController* BossController = GetBossController(&OwnerComp);
	ABoss* BossCharacter = GetBossCharacter(&OwnerComp);
	ACharacter* PlayerCharacter = GetPlayerCharacter(&OwnerComp);
	
	if (!BossController || !BossCharacter || !PlayerCharacter)
	{
		return EBTNodeResult::Failed;
	}
	
	// 플레이어 위치에 랜덤 오프셋 추가
	FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	
	// 랜덤 위치 오프셋 생성 (XY 평면상에서만)
	if (MaxRandomPositionOffset > 0.0f)
	{
		const float RandomRadius = FMath::RandRange(0.0f, MaxRandomPositionOffset);
		const float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
		
		FVector RandomOffset(
			FMath::Cos(RandomAngle) * RandomRadius,
			FMath::Sin(RandomAngle) * RandomRadius,
			0.0f
		);
		
		PlayerLocation += RandomOffset;
	}
	
	// 보스가 바라볼 방향 계산
	FVector DirectionToPlayer = PlayerLocation - BossCharacter->GetActorLocation();
	DirectionToPlayer.Normalize();
	
	// 랜덤 각도 오프셋 추가 (Yaw 축에만)
	if (MaxRandomAngleOffset > 0.0f)
	{
		float RandomYawOffset = FMath::RandRange(-MaxRandomAngleOffset, MaxRandomAngleOffset);
		FRotator RandomRotOffset(0.0f, RandomYawOffset, 0.0f);
		DirectionToPlayer = RandomRotOffset.RotateVector(DirectionToPlayer);
	}
	
	// 목표 회전값 계산 및 메모리에 저장
	Memory->TargetRotation = DirectionToPlayer.Rotation();
	
	// 현재 보스의 회전값 가져오기
	FRotator CurrentRotation = BossCharacter->GetActorRotation();
	
	// Pitch와 Roll은 유지 (Yaw만 변경)
	Memory->TargetRotation.Pitch = CurrentRotation.Pitch;
	Memory->TargetRotation.Roll = CurrentRotation.Roll;
	
	// 초기화 완료
	Memory->bInitialized = true;
	
	// Tick 함수에서 회전 처리
	return EBTNodeResult::InProgress;
}

void UBTTask_BossLookAtPlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBossLookAtPlayerMemory* Memory = reinterpret_cast<FBossLookAtPlayerMemory*>(NodeMemory);
	
	// 초기화 확인
	if (!Memory->bInitialized)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	// 보스 캐릭터 가져오기
	ABoss* BossCharacter = GetBossCharacter(&OwnerComp);
	if (!BossCharacter)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	// 현재 보스의 회전값
	FRotator CurrentRotation = BossCharacter->GetActorRotation();
	
	// 부드러운 회전을 위한 보간
	FRotator NewRotation = FMath::RInterpTo(
		CurrentRotation,
		Memory->TargetRotation,
		DeltaSeconds,
		RotationSpeed
	);
	
	// 회전 적용
	BossCharacter->SetActorRotation(NewRotation);
	
	// 회전 일치도 확인 (Yaw만 비교)
	float RotationMatchDegree = 1.0f - (FMath::Abs(FMath::FindDeltaAngleDegrees(NewRotation.Yaw, Memory->TargetRotation.Yaw)) / 180.0f);
	
	// 충분히 회전했으면 성공
	if (RotationMatchDegree >= MinRotationMatchThreshold)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
