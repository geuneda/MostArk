// Fill out your copyright notice in the Description page of Project Settings.

#include "STripodSystemWidget.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Engine/Texture2D.h"
#include "Styling/SlateTypes.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void STripodSystemWidget::Construct(const FArguments& InArgs)
{
    OwnerPlayer = InArgs._OwnerPlayer;

    ChildSlot
    [
        SNew(SBorder)
        .BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
        .Padding(FMargin(4.0f))
        [
            SNew(SVerticalBox)
            
            // 제목
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(FMargin(0, 0, 0, 10))
            .HAlign(HAlign_Center)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("트라이포드 시스템")))
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
            ]
            
            // 스킬 목록
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(FMargin(0, 0, 0, 10))
            [
                CreateSkillListWidget()
            ]
            
            // 트라이포드 티어 표시 영역
            + SVerticalBox::Slot()
            .FillHeight(1.0f)
            [
                SNew(SBorder)
                .BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.DarkGroupBorder"))
                .Padding(FMargin(8.0f))
                [
                    SNew(SVerticalBox)
                    
                    // 선택된 스킬 이름 표시
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(FMargin(0, 0, 0, 10))
                    .HAlign(HAlign_Center)
                    [
                        SNew(STextBlock)
                        .Text_Lambda([this]() -> FText {
                            if (OwnerPlayer.IsValid() && OwnerPlayer->SelectedSkillIndex < OwnerPlayer->Skills.Num())
                            {
                                return FText::FromString(OwnerPlayer->Skills[OwnerPlayer->SelectedSkillIndex].SkillName);
                            }
                            return FText::FromString(TEXT("스킬을 선택하세요"));
                        })
                        .Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
                    ]
                    
                    // 1단계 트라이포드
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(FMargin(0, 5))
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString(TEXT("1단계 트라이포드")))
                            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            SNew(SBox)
                            [
                                CreateTripodTierWidget(OwnerPlayer->SelectedSkillIndex, 0)
                            ]
                        ]
                    ]
                    
                    // 2단계 트라이포드
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(FMargin(0, 5))
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString(TEXT("2단계 트라이포드")))
                            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            SNew(SBox)
                            [
                                CreateTripodTierWidget(OwnerPlayer->SelectedSkillIndex, 1)
                            ]
                        ]
                    ]
                    
                    // 3단계 트라이포드
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(FMargin(0, 5))
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString(TEXT("3단계 트라이포드")))
                            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            SNew(SBox)
                            [
                                CreateTripodTierWidget(OwnerPlayer->SelectedSkillIndex, 2)
                            ]
                        ]
                    ]
                ]
            ]
        ]
    ];
}

TSharedRef<SWidget> STripodSystemWidget::CreateSkillListWidget()
{
    TSharedRef<SGridPanel> SkillGrid = SNew(SGridPanel);
    
    if (OwnerPlayer.IsValid())
    {
        const int32 SkillsPerRow = 4;
        
        // 각 스킬에 대한 버튼 생성
        for (int32 i = 0; i < OwnerPlayer->Skills.Num(); ++i)
        {
            int32 Row = i / SkillsPerRow;
            int32 Column = i % SkillsPerRow;
            
            FSkillData& Skill = OwnerPlayer->Skills[i];
            
            SkillGrid->AddSlot(Column, Row)
            .Padding(5)
            [
                SNew(SBox)
                .WidthOverride(80)
                .HeightOverride(100)
                [
                    SNew(SVerticalBox)
                    
                    // 스킬 버튼
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(SButton)
                        .OnClicked(this, &STripodSystemWidget::OnSkillSelected, i)
                        .HAlign(HAlign_Center)
                        .VAlign(VAlign_Center)
                        [
                            SNew(SBox)
                            .WidthOverride(64)
                            .HeightOverride(64)
                            [
                                SNew(SBorder)
                                .HAlign(HAlign_Center)
                                .VAlign(VAlign_Center)
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString(Skill.SkillName.Left(1)))
                                ]
                            ]
                        ]
                    ]
                    
                    // 스킬 이름
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .HAlign(HAlign_Center)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(Skill.SkillName))
                        .Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
                    ]
                    
                    // 레벨 표시
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .HAlign(HAlign_Center)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(FString::Printf(TEXT("Lv. %d"), Skill.SkillLevel)))
                        .Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
                    ]
                ]
            ];
        }
    }
    
    return SkillGrid;
}

TSharedRef<SWidget> STripodSystemWidget::CreateTripodTierWidget(int32 SkillIndex, int32 TierIndex)
{
    TSharedRef<SHorizontalBox> TierBox = SNew(SHorizontalBox);
    
    if (OwnerPlayer.IsValid() && OwnerPlayer->Skills.IsValidIndex(SkillIndex) && 
        OwnerPlayer->Skills[SkillIndex].TripodTiers.IsValidIndex(TierIndex))
    {
        FTripodTier& Tier = OwnerPlayer->Skills[SkillIndex].TripodTiers[TierIndex];
        
        // 티어가 잠겨있는 경우 잠금 표시
        if (!Tier.bIsUnlocked)
        {
            int32 unlockLevel = TierIndex == 0 ? 4 : (TierIndex == 1 ? 7 : 10);
            TierBox->AddSlot()
            .AutoWidth()
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text(FText::FromString(FString::Printf(TEXT("스킬 레벨 %d에 해제됩니다."), unlockLevel)))
                .Font(FCoreStyle::GetDefaultFontStyle("Italic", 12))
            ];
        }
        else
        {
            // 해당 티어의 각 효과 표시
            for (int32 i = 0; i < Tier.TripodEffects.Num(); ++i)
            {
                TierBox->AddSlot()
                .AutoWidth()
                .Padding(5, 0)
                [
                    CreateTripodEffectWidget(SkillIndex, TierIndex, i)
                ];
            }
        }
    }
    
    return TierBox;
}

TSharedRef<SWidget> STripodSystemWidget::CreateTripodEffectWidget(int32 SkillIndex, int32 TierIndex, int32 EffectIndex)
{
    if (OwnerPlayer.IsValid() && 
        OwnerPlayer->Skills.IsValidIndex(SkillIndex) &&
        OwnerPlayer->Skills[SkillIndex].TripodTiers.IsValidIndex(TierIndex) &&
        OwnerPlayer->Skills[SkillIndex].TripodTiers[TierIndex].TripodEffects.IsValidIndex(EffectIndex))
    {
        FTripodEffect& Effect = OwnerPlayer->Skills[SkillIndex].TripodTiers[TierIndex].TripodEffects[EffectIndex];
        bool bIsSelected = OwnerPlayer->Skills[SkillIndex].TripodTiers[TierIndex].SelectedEffectIndex == EffectIndex;
        
        return SNew(SBox)
        .WidthOverride(80)
        .HeightOverride(100)
        [
            SNew(SButton)
            .OnClicked(this, &STripodSystemWidget::OnTripodEffectSelected, SkillIndex, TierIndex, EffectIndex)
            [
                SNew(SBorder)
                .BorderImage(FCoreStyle::Get().GetBrush(bIsSelected ? "Button.Pressed" : "Button.Normal"))
                .Padding(FMargin(4.0f))
                [
                    SNew(SVerticalBox)
                    
                    // 효과 아이콘
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .HAlign(HAlign_Center)
                    [
                        SNew(SBox)
                        .WidthOverride(60)
                        .HeightOverride(60)
                        [
                            SNew(SBorder)
                            .BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
                            .HAlign(HAlign_Center)
                            .VAlign(VAlign_Center)
                            [
                                SNew(STextBlock)
                                .Text(FText::FromString(TEXT("효과")))
                            ]
                        ]
                    ]
                    
                    // 효과 이름
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .HAlign(HAlign_Center)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(Effect.EffectName))
                        .Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
                    ]
                ]
            ]
        ];
    }
    
    // 효과가 없는 경우 빈 위젯 반환
    return SNew(SBox)
    .WidthOverride(80)
    .HeightOverride(100);
}

FReply STripodSystemWidget::OnSkillSelected(int32 SkillIndex)
{
    if (OwnerPlayer.IsValid() && OwnerPlayer->Skills.IsValidIndex(SkillIndex))
    {
        OwnerPlayer->SelectedSkillIndex = SkillIndex;
    }
    
    return FReply::Handled();
}

FReply STripodSystemWidget::OnTripodEffectSelected(int32 SkillIndex, int32 TierIndex, int32 EffectIndex)
{
    if (OwnerPlayer.IsValid())
    {
        OwnerPlayer->SelectTripod(SkillIndex, TierIndex, EffectIndex);
    }
    
    return FReply::Handled();
}

FReply STripodSystemWidget::OnLevelUpButtonClicked(int32 SkillIndex)
{
    if (OwnerPlayer.IsValid())
    {
        OwnerPlayer->LevelUpSkill(SkillIndex);
    }
    
    return FReply::Handled();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION