// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_BossChasePlayer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "../Boss.h"
#include "GameFramework/Character.h"

UBTTask_BossChasePlayer::UBTTask_BossChasePlayer()
{
    // 태스크가 Tick을 사용할 수 있도록 설정
    bCreateNodeInstance = true;
    bNotifyTick = true;
    
    // 노드 이름 설정
    NodeName = TEXT("플레이어 추적");
}

EBTNodeResult::Type UBTTask_BossChasePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // 보스 컨트롤러 가져오기
    AAIController* BossController = GetBossController(&OwnerComp);
    if (!BossController)
    {
        return EBTNodeResult::Failed;
    }
    
    // 블랙보드 가져오기
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return EBTNodeResult::Failed;
    }
    
    // 플레이어 캐릭터 가져오기
    ACharacter* PlayerCharacter = Cast<ACharacter>(BlackboardComp->GetValueAsObject("TargetPlayer"));
    if (!PlayerCharacter)
    {
        return EBTNodeResult::Failed;
    }
    
    // 플레이어 위치 업데이트
    BlackboardComp->SetValueAsVector("TargetLocation", PlayerCharacter->GetActorLocation());
    
    // 플레이어 위치로 이동 명령
    BossController->MoveToActor(PlayerCharacter, AcceptanceRadius);
    
    // 마지막 업데이트 시간 초기화
    LastUpdateTime = 0.0f;
    
    // 태스크가 계속 실행 중임을 알림
    return EBTNodeResult::InProgress;
}

void UBTTask_BossChasePlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    // 경과 시간 업데이트
    LastUpdateTime += DeltaSeconds;
    
    // 업데이트 간격이 지났는지 확인
    if (LastUpdateTime >= UpdateInterval)
    {
        // 보스 컨트롤러 가져오기
        AAIController* BossController = GetBossController(&OwnerComp);
        if (!BossController)
        {
            FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
            return;
        }
        
        // 블랙보드 가져오기
        UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
        if (!BlackboardComp)
        {
            FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
            return;
        }
        
        // 플레이어 캐릭터 가져오기
        ACharacter* PlayerCharacter = Cast<ACharacter>(BlackboardComp->GetValueAsObject("TargetPlayer"));
        if (!PlayerCharacter)
        {
            FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
            return;
        }
        
        // 플레이어 위치 업데이트
        BlackboardComp->SetValueAsVector("TargetLocation", PlayerCharacter->GetActorLocation());
        
        // 마지막 업데이트 시간 초기화
        LastUpdateTime = 0.0f;
    }
    
    // 보스 캐릭터 가져오기
    ABoss* Boss = GetBossCharacter(&OwnerComp);
    if (!Boss)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    // 블랙보드 가져오기
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    // 플레이어 캐릭터 가져오기
    ACharacter* PlayerCharacter = Cast<ACharacter>(BlackboardComp->GetValueAsObject("TargetPlayer"));
    if (!PlayerCharacter)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    // 보스와 플레이어 간의 거리 계산
    float DistanceToPlayer = FVector::Dist(Boss->GetActorLocation(), PlayerCharacter->GetActorLocation());
    
    // 공격 가능 범위 가져오기
    float AttackRange = BlackboardComp->GetValueAsFloat("AttackRange");
    
    // 플레이어가 공격 범위 내에 있는지 확인
    if (DistanceToPlayer <= AttackRange)
    {
        // 공격 범위 내에 있으면 블랙보드 값 업데이트
        BlackboardComp->SetValueAsBool("IsInAttackRange", true);
        
    }
        // 태스크 완료
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}
