// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_BossGroundAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../Boss.h"
#include "../BossAnimInstance.h"
#include "GameFramework/Character.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"

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
    
    // VFX 생성 위치 처리 - 보스 앞쪽 바닥
    FVector BossLocation = Boss->GetActorLocation();
    
    // 그라운드 공격 VFX 스폰
    GroundAttackVFXComponent = Boss->SpawnGroundAttackVFX(BossLocation);
    
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
            // VFX 정리
            CleanupVFX();
            FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
            return;
        }
        
        // 보스 애니메이션 인스턴스 가져오기
        UBossAnimInstance* BossAnim = Cast<UBossAnimInstance>(Boss->GetMesh()->GetAnimInstance());
        if (!BossAnim)
        {
            // VFX 정리
            CleanupVFX();
            FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
            return;
        }
        
        // 그라운드 공격 몽타주 정지
        BossAnim->StopGroundAttackMontage();
        
        // 종료 VFX 생성
        GroundAttackFinishVFXComponent = Boss->SpawnGroundAttackFinishVFX(VFXSpawnLocation);
        
        // 시작 VFX 정리
        CleanupVFX();
        
        // 1초 뒤에 종료 VFX 정리
        FTimerHandle FinishVFXTimerHandle;
        GetWorld()->GetTimerManager().SetTimer(FinishVFXTimerHandle, [this]()
        {
            if (GroundAttackFinishVFXComponent)
            {
                GroundAttackFinishVFXComponent->SetFloatParameter(TEXT("FadeOutMultiplier"), 1.0f);
                GroundAttackFinishVFXComponent->SetAutoDestroy(true);
                GroundAttackFinishVFXComponent = nullptr;
            }
        }, 1.0f, false);
        
        // 태스크 완료
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}

// VFX 정리 함수
void UBTTask_BossGroundAttack::CleanupVFX()
{
    if (GroundAttackVFXComponent)
    {
        // 페이드아웃 효과를 적용하여 서서히 사라지게 함
        GroundAttackVFXComponent->SetFloatParameter(TEXT("FadeOutMultiplier"), 1.0f);
        
        // 2초 후 자동 제거되도록 설정
        GroundAttackVFXComponent->SetAutoDestroy(true);
        
        // 참조 초기화
        GroundAttackVFXComponent = nullptr;
    }
}
