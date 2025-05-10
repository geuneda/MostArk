// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_BossSelectAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "../Boss.h"
#include "../BossAnimInstance.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UBTTask_BossSelectAttack::UBTTask_BossSelectAttack()
{
    // 노드 이름 설정
    NodeName = TEXT("공격 방식 선택");
}

EBTNodeResult::Type UBTTask_BossSelectAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // 보스 캐릭터 가져오기
    ABoss* Boss = GetBossCharacter(&OwnerComp);
    if (!Boss)
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
    
    // 보스 애니메이션 인스턴스 가져오기
    UBossAnimInstance* BossAnim = Cast<UBossAnimInstance>(Boss->GetMesh()->GetAnimInstance());
    if (!BossAnim)
    {
        return EBTNodeResult::Failed;
    }
    
    // 플레이어와의 각도 계산
    float AngleToPlayer = CalculateAngleToPlayer(Boss, PlayerCharacter);
    
    // 블랙보드에 각도 저장
    BlackboardComp->SetValueAsFloat("AngleToPlayer", AngleToPlayer);
    
    // 각도에 따라 공격 방식 선택
    if (AngleToPlayer >= -FrontAttackAngleThreshold && AngleToPlayer <= FrontAttackAngleThreshold)
    {
        // 정면 공격 (왼손, 꼬리, 오른손 중 하나 선택)
        if (AngleToPlayer <= LeftAttackAngleThreshold)
        {
            // 왼손 공격
            BlackboardComp->SetValueAsEnum("AttackType", 0); // 0: 왼손 공격
            BossAnim->PlayLeftAttackMontage();
        }
        else if (AngleToPlayer <= TailAttackAngleThreshold)
        {
            // 꼬리 공격
            BlackboardComp->SetValueAsEnum("AttackType", 1); // 1: 꼬리 공격
            BossAnim->PlayTailAttackMontage();
        }
        else
        {
            // 오른손 공격
            BlackboardComp->SetValueAsEnum("AttackType", 2); // 2: 오른손 공격
            BossAnim->PlayRightAttackMontage();
        }
        
        return EBTNodeResult::Succeeded;
    }
    else if (AngleToPlayer < -FrontAttackAngleThreshold && AngleToPlayer >= -SideAttackAngleThreshold)
    {
        // 좌측 이동 공격 (아직 구현되지 않음)
        BlackboardComp->SetValueAsEnum("AttackType", 3); // 3: 좌측 이동 공격
        
        // 좌측 이동 공격 몽타주 재생 (아직 구현되지 않음)
        // BossAnim->PlayLeftMoveAttackMontage();
        
        // 몽타주가 구현되지 않았으므로 일단 왼손 공격으로 대체
        BossAnim->PlayLeftAttackMontage();
        
        return EBTNodeResult::Succeeded;
    }
    else if (AngleToPlayer > FrontAttackAngleThreshold && AngleToPlayer <= SideAttackAngleThreshold)
    {
        // 우측 이동 공격 (아직 구현되지 않음)
        BlackboardComp->SetValueAsEnum("AttackType", 4); // 4: 우측 이동 공격
        
        // 우측 이동 공격 몽타주 재생 (아직 구현되지 않음)
        // BossAnim->PlayRightMoveAttackMontage();
        
        // 몽타주가 구현되지 않았으므로 일단 오른손 공격으로 대체
        BossAnim->PlayRightAttackMontage();
        
        return EBTNodeResult::Succeeded;
    }
    else
    {
        // 플레이어가 뒤에 있는 경우 공격하지 않고 BT를 처음부터 다시 시작
        return EBTNodeResult::Failed;
    }
}

float UBTTask_BossSelectAttack::CalculateAngleToPlayer(ABoss* Boss, ACharacter* Player)
{
    if (!Boss || !Player)
    {
        return 0.0f;
    }
    
    // 보스의 전방 벡터
    FVector BossForward = Boss->GetActorForwardVector();
    
    // 보스에서 플레이어로의 방향 벡터
    FVector DirectionToPlayer = Player->GetActorLocation() - Boss->GetActorLocation();
    DirectionToPlayer.Z = 0.0f; // 높이 차이 무시
    DirectionToPlayer.Normalize();
    
    // 두 벡터 사이의 각도 계산 (내적 이용)
    float DotProduct = FVector::DotProduct(BossForward, DirectionToPlayer);
    
    // 각도 계산 (라디안에서 디그리로 변환)
    float AngleRadians = FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f));
    float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);
    
    // 왼쪽/오른쪽 판단을 위한 외적
    FVector CrossProduct = FVector::CrossProduct(BossForward, DirectionToPlayer);
    
    // 외적의 Z 값이 음수면 플레이어는 보스의 오른쪽에 있음 (양수 각도)
    // 외적의 Z 값이 양수면 플레이어는 보스의 왼쪽에 있음 (음수 각도)
    return CrossProduct.Z < 0 ? AngleDegrees : -AngleDegrees;
}
