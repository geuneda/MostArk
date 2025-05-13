// Fill out your copyright notice in the Description page of Project Settings.

#include "BTDecorator_RandomActionSelect.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

UBTDecorator_RandomActionSelect::UBTDecorator_RandomActionSelect()
{
    // 노드 이름 설정
    NodeName = TEXT("랜덤 행동 선택");
    
    // 블랙보드 키 초기화
    LastSpecialPatternTimeKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_RandomActionSelect, LastSpecialPatternTimeKey));
    CanUseSpecialPatternKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_RandomActionSelect, CanUseSpecialPatternKey));
}

void UBTDecorator_RandomActionSelect::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);
    
    // 블랙보드 키 초기화
    UBlackboardData* BBAsset = GetBlackboardAsset();
    if (BBAsset)
    {
        LastSpecialPatternTimeKey.ResolveSelectedKey(*BBAsset);
        CanUseSpecialPatternKey.ResolveSelectedKey(*BBAsset);
    }
}

bool UBTDecorator_RandomActionSelect::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    // 블랙보드 컴포넌트 가져오기
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return false;
    }
    
    // 현재 시간 가져오기
    const float CurrentTime = UGameplayStatics::GetTimeSeconds(OwnerComp.GetOwner());
    
    // 마지막 특수 패턴 사용 시간 가져오기
    const float LastSpecialPatternTime = BlackboardComp->GetValueAsFloat(LastSpecialPatternTimeKey.SelectedKeyName);
    
    // 쿨다운 시간이 지났는지 확인
    const bool bCooldownElapsed = (CurrentTime - LastSpecialPatternTime) >= CooldownTime;
    
    // 쿨다운 상태를 블랙보드에 저장
    BlackboardComp->SetValueAsBool(CanUseSpecialPatternKey.SelectedKeyName, bCooldownElapsed);
    
    // 쿨다운 중이면 특수 패턴 사용 불가
    if (!bCooldownElapsed)
    {
        return false;
    }
    
    // 확률에 따라 특수 패턴 사용 여부 결정
    const float RandomValue = FMath::FRand(); // 0.0 ~ 1.0 사이의 랜덤 값
    const bool bUseSpecialPattern = RandomValue <= SpecialPatternProbability;
    
    // 특수 패턴을 사용하기로 결정했다면 마지막 사용 시간 업데이트
    if (bUseSpecialPattern)
    {
        BlackboardComp->SetValueAsFloat(LastSpecialPatternTimeKey.SelectedKeyName, CurrentTime);
    }
    
    // 특수 패턴 사용 여부 반환
    return bUseSpecialPattern;
}
