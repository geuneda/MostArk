// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_DetectPlayer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../Boss.h"
#include "GameFramework/Character.h"

UBTTask_DetectPlayer::UBTTask_DetectPlayer()
{
    // 노드 이름 설정
    NodeName = TEXT("플레이어 감지");
}

EBTNodeResult::Type UBTTask_DetectPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // 보스 캐릭터 가져오기
    ABoss* Boss = GetBossCharacter(&OwnerComp);
    if (!Boss)
    {
        return EBTNodeResult::Failed;
    }
    
    // 플레이어 캐릭터 가져오기
    ACharacter* PlayerCharacter = GetPlayerCharacter(&OwnerComp);
    if (!PlayerCharacter)
    {
        return EBTNodeResult::Failed;
    }
    
    // 블랙보드 가져오기
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return EBTNodeResult::Failed;
    }
    
    // 보스와 플레이어 간의 거리 계산
    float DistanceToPlayer = FVector::Dist(Boss->GetActorLocation(), PlayerCharacter->GetActorLocation());
    
    // 플레이어가 감지 범위 내에 있는지 확인
    bool bIsPlayerDetected = DistanceToPlayer <= DetectionRadius;
    
    // 블랙보드에 플레이어 감지 여부 저장
    BlackboardComp->SetValueAsBool("IsPlayerDetected", bIsPlayerDetected);
    
    // 플레이어가 감지되었다면 플레이어 위치도 저장
    if (bIsPlayerDetected)
    {
        BlackboardComp->SetValueAsObject("TargetPlayer", PlayerCharacter);
        BlackboardComp->SetValueAsVector("TargetLocation", PlayerCharacter->GetActorLocation());
    }
    
    return EBTNodeResult::Succeeded;
}
