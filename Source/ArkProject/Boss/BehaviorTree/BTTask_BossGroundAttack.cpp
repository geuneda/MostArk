// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_BossGroundAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../Boss.h"
#include "../BossAnimInstance.h"
#include "GameFramework/Character.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"

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
    
    // 보스 컨트롤러 가져오기
    AAIController* BossController = GetBossController(&OwnerComp);
    if (!BossController)
    {
        return EBTNodeResult::Failed;
    }
    
    // 보스 애니메이션 인스턴스 가져오기
    UBossAnimInstance* BossAnim = Cast<UBossAnimInstance>(Boss->GetMesh()->GetAnimInstance());
    if (!BossAnim)
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
    
    // 그라운드 공격 몽타주 재생
    BossAnim->PlayGroundAttackMontage();
    
    // 상태 초기화
    CurrentTime = 0.0f;
    LastUpdateTime = 0.0f;
    bAttackStarted = true;
    bIsFollowingPlayer = true;
    
    // VFX 생성 위치 처리 - 보스 앞쪽 바닥
    FVector BossLocation = Boss->GetActorLocation();
    
    // 그라운드 공격 VFX 스폰
    GroundAttackVFXComponent = Boss->SpawnGroundAttackVFX(BossLocation);
    
    BossController->MoveToActor(PlayerCharacter, AcceptanceRadius);
    // 태스크가 계속 실행 중임을 알림
    return EBTNodeResult::InProgress;
}

void UBTTask_BossGroundAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    // 경과 시간 업데이트
    CurrentTime += DeltaSeconds;
    LastUpdateTime += DeltaSeconds;
    
    // 보스 캐릭터와 컨트롤러 가져오기
    ABoss* Boss = GetBossCharacter(&OwnerComp);
    AAIController* BossController = GetBossController(&OwnerComp);
    
    if (!Boss || !BossController)
    {
        CleanupVFX();
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    // 플레이어 추적 중이고 업데이트 시간이 되었다면 플레이어 위치 업데이트
    if (bIsFollowingPlayer && LastUpdateTime >= UpdateInterval)
    {
        // 블랙보드 가져오기
        UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
        if (BlackboardComp)
        {
            // 플레이어 캐릭터 가져오기
            ACharacter* PlayerCharacter = Cast<ACharacter>(BlackboardComp->GetValueAsObject("TargetPlayer"));
            if (PlayerCharacter)
            {
                // 플레이어 위치로 이동 명령 업데이트
                BossController->MoveToActor(PlayerCharacter, AcceptanceRadius);
            }
        }
        
        // 마지막 업데이트 시간 초기화
        LastUpdateTime = 0.0f;
    }
    
    // 추적 중지 시간에 도달했는지 확인
    if (bIsFollowingPlayer && CurrentTime >= StopFollowingTime)
    {
        // 플레이어 추적 중지
        BossController->StopMovement();
        bIsFollowingPlayer = false;
        
        UE_LOG(LogTemp, Warning, TEXT("Boss - 플레이어 추적 중지: %f초"), CurrentTime);
    }
    
    // 공격 지속 시간이 지났는지 확인
    if (CurrentTime >= GroundAttackDuration)
    {
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
