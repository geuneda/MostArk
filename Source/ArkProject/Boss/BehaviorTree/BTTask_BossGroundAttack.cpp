// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_BossGroundAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "../Boss.h"
#include "../BossAnimInstance.h"
#include "GameFramework/Character.h"

UBTTask_BossGroundAttack::UBTTask_BossGroundAttack()
{
    // 태스크가 Tick을 사용할 수 있도록 설정
    bCreateNodeInstance = true;
    bNotifyTick = true;
    
    // 노드 이름 설정
    NodeName = TEXT("그라운드 공격 패턴");
}

EBTNodeResult::Type UBTTask_BossGroundAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // 보스 캐릭터 가져오기
    ABoss* Boss = GetBossCharacter(&OwnerComp);
    if (!Boss)
    {
        return EBTNodeResult::Failed;
    }
    
    // 보스 애니메이션 인스턴스 가져오기
    UBossAnimInstance* BossAnim = Cast<UBossAnimInstance>(Boss->GetMesh()->GetAnimInstance());
    if (!BossAnim)
    {
        return EBTNodeResult::Failed;
    }
    
    // 그라운드 공격 몽타주 재생
    BossAnim->PlayGroundAttackMontage();
    
    // 상태 초기화
    CurrentTime = 0.0f;
    bAttackStarted = true;
    
    // 태스크가 계속 실행 중임을 알림
    return EBTNodeResult::InProgress;
}

void UBTTask_BossGroundAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    // 경과 시간 업데이트
    CurrentTime += DeltaSeconds;
    
    // 공격 지속 시간이 지났는지 확인
    if (CurrentTime >= GroundAttackDuration)
    {
        // 보스 캐릭터 가져오기
        ABoss* Boss = GetBossCharacter(&OwnerComp);
        if (!Boss)
        {
            FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
            return;
        }
        
        // 보스 애니메이션 인스턴스 가져오기
        UBossAnimInstance* BossAnim = Cast<UBossAnimInstance>(Boss->GetMesh()->GetAnimInstance());
        if (!BossAnim)
        {
            FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
            return;
        }
        
        // 그라운드 공격 몽타주 정지
        BossAnim->StopGroundAttackMontage();
        
        // 태스크 완료
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}
