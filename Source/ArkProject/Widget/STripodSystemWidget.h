// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "../Player/MostArkPlayer.h"

/**
 * 트라이포드 시스템 Slate UI 위젯
 */
class ARKPROJECT_API STripodSystemWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STripodSystemWidget)
        : _OwnerPlayer(nullptr)
    {}
        SLATE_ARGUMENT(AMostArkPlayer*, OwnerPlayer)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    
    /** UI 갱신 메서드 */
    void RefreshWidget();

private:
    /** 플레이어 */
    TWeakObjectPtr<AMostArkPlayer> OwnerPlayer;
    
    /** 스킬 그리드 위젯 */
    TSharedPtr<class SGridPanel> SkillGridWidget;

    /** 스킬 목록 위젯 생성 */
    void CreateSkillListWidget();

    /** 트라이포드 티어 위젯 생성 */
    TSharedRef<SWidget> CreateTripodTierWidget(int32 SkillIndex, int32 TierIndex);

    /** 트라이포드 효과 위젯 생성 */
    TSharedRef<SWidget> CreateTripodEffectWidget(int32 SkillIndex, int32 TierIndex, int32 EffectIndex);

    /** 스킬 선택 콜백 */
    FReply OnSkillSelected(int32 SkillIndex);

    /** 트라이포드 효과 선택 콜백 */
    FReply OnTripodEffectSelected(int32 SkillIndex, int32 TierIndex, int32 EffectIndex);

    /** 레벨업 버튼 클릭 콜백 */
    FReply OnLevelUpButtonClicked(int32 SkillIndex);

    /** 레벨다운 버튼 클릭 콜백 */
    FReply OnLevelDownButtonClicked(int32 SkillIndex);
};