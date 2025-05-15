// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerSkillWidget.h"
#include "Components/TextBlock.h"

void UPlayerSkillWidget::UpdateSkillCooldownText(int32 SkillIndex, float RemainingTime)
{
    // 소수점 첫째 자리까지 표시
    FString CooldownText = FString::Printf(TEXT("%.1f"), RemainingTime);
    
    switch (SkillIndex)
    {
    case 0:
        if (Skill1Text)
        {
            Skill1Text->SetText(FText::FromString(CooldownText));
        }
        break;
    case 1:
        if (Skill2Text)
        {
            Skill2Text->SetText(FText::FromString(CooldownText));
        }
        break;
    case 2:
        if (Skill3Text)
        {
            Skill3Text->SetText(FText::FromString(CooldownText));
        }
        break;
    default:
        break;
    }
}

void UPlayerSkillWidget::ResetSkillCooldownText(int32 SkillIndex)
{
    // 쿨다운이 끝나면 텍스트를 빈 문자열로 설정
    switch (SkillIndex)
    {
    case 0:
        if (Skill1Text)
        {
            Skill1Text->SetText(FText::FromString(TEXT("")));
        }
        break;
    case 1:
        if (Skill2Text)
        {
            Skill2Text->SetText(FText::FromString(TEXT("")));
        }
        break;
    case 2:
        if (Skill3Text)
        {
            Skill3Text->SetText(FText::FromString(TEXT("")));
        }
        break;
    default:
        break;
    }
}
