// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_BossPatrol.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "../Boss.h"

UBTTask_BossPatrol::UBTTask_BossPatrol()
{
    // 태스크가 Tick을 사용할 수 있도록 설정
    bCreateNodeInstance = true;
    bNotifyTick = true;
    bNotifyTaskFinished = true;
    
    // 노드 이름 설정
    NodeName = TEXT("보스 정찰");
}

EBTNodeResult::Type UBTTask_BossPatrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // 보스 컨트롤러 가져오기
    AAIController* BossController = GetBossController(&OwnerComp);
    if (!BossController)
    {
        return EBTNodeResult::Failed;
    }
    
    // 대기 시간 초기화
    CurrentWaitTime = 0.0f;
    bHasReachedDestination = false;
    
    // 새로운 정찰 지점 찾기
    if (FindNewPatrolPoint(OwnerComp))
    {
        return EBTNodeResult::InProgress;
    }
    
    return EBTNodeResult::Failed;
}

void UBTTask_BossPatrol::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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
    
    // 목적지 위치 가져오기
    FVector TargetLocation = BlackboardComp->GetValueAsVector("PatrolLocation");
    
    // 보스 캐릭터 가져오기
    ABoss* Boss = GetBossCharacter(&OwnerComp);
    if (!Boss)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    // **여기까지 문제 없음
    
    // 현재 위치와 목적지 간의 거리 계산
    FVector CurrentLocation = Boss->GetActorLocation();
    float DistanceToTarget = FVector::Dist(CurrentLocation, TargetLocation);

    UE_LOG(LogTemp, Warning, TEXT("DistanceToTarget: %f"), DistanceToTarget);
    
    // 목적지에 도달했는지 확인
    if (!bHasReachedDestination )
    {
        bHasReachedDestination = true;
    }
    
    // 목적지에 도달했다면 대기 시간 카운트
    if (bHasReachedDestination)
    {
        CurrentWaitTime += DeltaSeconds;
        
        // 대기 시간이 지났다면 새로운 정찰 지점 찾기
        if (CurrentWaitTime >= WaitTime)
        {
            if (FindNewPatrolPoint(OwnerComp))
            {
                // 상태 초기화
                CurrentWaitTime = 0.0f;
                bHasReachedDestination = false;
            }
                FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        }
    }
}

void UBTTask_BossPatrol::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    // 태스크가 완료되면 필요한 정리 작업 수행
}

bool UBTTask_BossPatrol::FindNewPatrolPoint(UBehaviorTreeComponent& OwnerComp)
{
    // 보스 캐릭터 가져오기
    ABoss* Boss = GetBossCharacter(&OwnerComp);
    if (!Boss)
    {
        return false;
    }
    
    // 보스 컨트롤러 가져오기
    AAIController* BossController = GetBossController(&OwnerComp);
    if (!BossController)
    {
        return false;
    }
    
    // 내비게이션 시스템 가져오기
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(Boss->GetWorld());
    if (!NavSystem)
    {
        return false;
    }
    
    // 블랙보드 가져오기
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return false;
    }
    
    // 현재 위치 주변에서 랜덤한 위치 찾기
    FNavLocation RandomLocation;
    bool bFoundLocation = NavSystem->GetRandomReachablePointInRadius(
        Boss->GetActorLocation(),
        PatrolRadius,
        RandomLocation
    );
    
    // 위치를 찾았다면 블랙보드에 저장하고 이동 명령 실행
    if (bFoundLocation)
    {
        BlackboardComp->SetValueAsVector("PatrolLocation", RandomLocation.Location);
        BossController->MoveToLocation(RandomLocation.Location, AcceptanceRadius);
        
        UE_LOG(LogTemp, Warning, TEXT("New Patrol Location: %s"), *RandomLocation.Location.ToString());
        
        return true;
    }
    
    return false;
}
