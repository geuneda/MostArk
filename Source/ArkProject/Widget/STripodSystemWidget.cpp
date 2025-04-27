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

void STripodSystemWidget::Construct(const FArguments &InArgs)
{
    OwnerPlayer = InArgs._OwnerPlayer;

    // 스킬 그리드 위젯 초기화
    SkillGridWidget = SNew(SGridPanel);

    // 초기 스킬 리스트 생성
    CreateSkillListWidget();

    ChildSlot
        [SNew(SBorder)
             .BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
             .Padding(FMargin(4.0f))
                 [SNew(SVerticalBox)

                  // 제목
                  + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(FMargin(0, 0, 0, 10))
                        .HAlign(HAlign_Center)
                            [SNew(STextBlock)
                                 .Text(FText::FromString(TEXT("트라이포드 시스템")))
                                 .Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))]

                  // 스킬 목록
                  + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(FMargin(0, 0, 0, 10))
                            [SNew(SBox)
                                 [SkillGridWidget.ToSharedRef()]]

                  // 트라이포드 티어 표시 영역
                  + SVerticalBox::Slot()
                        .FillHeight(1.0f)
                            [SNew(SBorder)
                                 .BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.DarkGroupBorder"))
                                 .Padding(FMargin(8.0f))
                                     [SNew(SVerticalBox)

                                      // 선택된 스킬 이름 표시
                                      + SVerticalBox::Slot()
                                            .AutoHeight()
                                            .Padding(FMargin(0, 0, 0, 10))
                                            .HAlign(HAlign_Center)
                                                [SNew(STextBlock)
                                                     .Text_Lambda([this]() -> FText
                                                                  {
        if (OwnerPlayer.IsValid() && OwnerPlayer->SelectedSkillIndex < OwnerPlayer->Skills.Num())
        {
            return FText::FromString(OwnerPlayer->Skills[OwnerPlayer->SelectedSkillIndex].SkillName);
        }
        return FText::FromString(TEXT("스킬을 선택하세요")); })
                                                     .Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))]

                                      // 1단계 트라이포드 - 람다를 이용해 선택된 스킬의 트라이포드 정보를 실시간으로 생성
                                      + SVerticalBox::Slot()
                                            .AutoHeight()
                                            .Padding(FMargin(0, 5))
                                                [SNew(SVerticalBox) + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(TEXT("1단계 트라이포드"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))] + SVerticalBox::Slot().AutoHeight()[SAssignNew(Tripod1Slot, SBox)]]

                                      // 2단계 트라이포드 - 람다를 이용해 선택된 스킬의 트라이포드 정보를 실시간으로 생성
                                      + SVerticalBox::Slot()
                                            .AutoHeight()
                                            .Padding(FMargin(0, 5))
                                                [SNew(SVerticalBox) + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(TEXT("2단계 트라이포드"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))] + SVerticalBox::Slot().AutoHeight()[SAssignNew(Tripod2Slot, SBox)]]

                                      // 3단계 트라이포드 - 람다를 이용해 선택된 스킬의 트라이포드 정보를 실시간으로 생성
                                      + SVerticalBox::Slot()
                                            .AutoHeight()
                                            .Padding(FMargin(0, 5))
                                                [SNew(SVerticalBox) + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(TEXT("3단계 트라이포드"))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))] + SVerticalBox::Slot().AutoHeight()[SAssignNew(Tripod3Slot, SBox)]]]]]];

    // 초기 화면 갱신
    RefreshWidget();
}

void STripodSystemWidget::CreateSkillListWidget()
{
    if (!SkillGridWidget.IsValid() || !OwnerPlayer.IsValid())
        return;

    // 기존 그리드 위젯의 슬롯 비우기
    SkillGridWidget->ClearChildren();

    const int32 SkillsPerRow = 4;

    // 각 스킬에 대한 버튼 생성
    for (int32 i = 0; i < OwnerPlayer->Skills.Num(); ++i)
    {
        int32 Row = i / SkillsPerRow;
        int32 Column = i % SkillsPerRow;

        FSkillData &Skill = OwnerPlayer->Skills[i];

        SkillGridWidget->AddSlot(Column, Row)
            .Padding(5)
                [SNew(SBox)
                     .WidthOverride(80)
                     .HeightOverride(140)
                         [SNew(SVerticalBox)

                          // 스킬 버튼
                          + SVerticalBox::Slot()
                                .AutoHeight()
                                    [SNew(SButton)
                                         .OnClicked(this, &STripodSystemWidget::OnSkillSelected, i)
                                         .HAlign(HAlign_Center)
                                         .VAlign(VAlign_Center)
                                             [SNew(SBox)
                                                  .WidthOverride(64)
                                                  .HeightOverride(64)
                                                      [SNew(SBorder)
                                                           .HAlign(HAlign_Center)
                                                           .VAlign(VAlign_Center)
                                                               [SNew(STextBlock)
                                                                    .Text(FText::FromString(Skill.SkillName.Left(1)))]]]]

                          // 스킬 이름
                          + SVerticalBox::Slot()
                                .AutoHeight()
                                .HAlign(HAlign_Center)
                                    [SNew(STextBlock)
                                         .Text(FText::FromString(Skill.SkillName))
                                         .Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))]

                          // 레벨 표시
                          + SVerticalBox::Slot()
                                .AutoHeight()
                                .HAlign(HAlign_Center)
                                    [SNew(STextBlock)
                                         .Text(FText::FromString(FString::Printf(TEXT("Lv. %d"), Skill.SkillLevel)))
                                         .Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))]

                          // 스킬 포인트 표시
                          + SVerticalBox::Slot()
                                .AutoHeight()
                                .HAlign(HAlign_Center)
                                .Padding(FMargin(0, 2))
                                    [SNew(STextBlock)
                                         .Text_Lambda([this]() -> FText
                                                      {
                        if (OwnerPlayer.IsValid())
                        {
                            return FText::FromString(FString::Printf(TEXT("포인트: %d"), OwnerPlayer->SkillPoints));
                        }
                        return FText::FromString(TEXT("포인트: 0")); })
                                         .Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))]

                          // 레벨 조정 버튼들 (가로로 배치)
                          + SVerticalBox::Slot()
                                .AutoHeight()
                                .HAlign(HAlign_Center)
                                .Padding(FMargin(0, 2))
                                    [SNew(SHorizontalBox)

                                     // 레벨다운 버튼
                                     + SHorizontalBox::Slot()
                                           .AutoWidth()
                                           .Padding(FMargin(2, 0))
                                               [SNew(SButton)
                                                    .OnClicked(this, &STripodSystemWidget::OnLevelDownButtonClicked, i)
                                                    .ContentPadding(FMargin(4, 2))
                                                    .ButtonColorAndOpacity(FLinearColor(0.8f, 0.2f, 0.2f, 1.0f))
                                                    .IsEnabled_Lambda([this, i]() -> bool
                                                                      { return OwnerPlayer.IsValid() && OwnerPlayer->Skills[i].SkillLevel > 1; })
                                                        [SNew(STextBlock)
                                                             .Text(FText::FromString(TEXT("-")))
                                                             .Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))]]

                                     // 레벨업 버튼
                                     + SHorizontalBox::Slot()
                                           .AutoWidth()
                                           .Padding(FMargin(2, 0))
                                               [SNew(SButton)
                                                    .OnClicked(this, &STripodSystemWidget::OnLevelUpButtonClicked, i)
                                                    .ContentPadding(FMargin(4, 2))
                                                    .ButtonColorAndOpacity(FLinearColor(0.2f, 0.6f, 0.2f, 1.0f))
                                                    .IsEnabled_Lambda([this, i]() -> bool
                                                                      { return OwnerPlayer.IsValid() && OwnerPlayer->SkillPoints > 0; })
                                                        [SNew(STextBlock)
                                                             .Text(FText::FromString(TEXT("+")))
                                                             .Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))]]]]];
    }
}

void STripodSystemWidget::RefreshWidget()
{
    UE_LOG(LogTemp, Warning, TEXT("STripodSystemWidget::RefreshWidget 호출됨"));

    if (SkillGridWidget.IsValid())
    {
        // 스킬 목록 위젯 갱신
        CreateSkillListWidget();

        if (OwnerPlayer.IsValid())
        {
            // 각 티어별 트라이포드 위젯 갱신
            if (Tripod1Slot.IsValid())
            {
                Tripod1Slot->SetContent(CreateTripodTierWidget(OwnerPlayer->SelectedSkillIndex, 0));
            }

            if (Tripod2Slot.IsValid())
            {
                Tripod2Slot->SetContent(CreateTripodTierWidget(OwnerPlayer->SelectedSkillIndex, 1));
            }

            if (Tripod3Slot.IsValid())
            {
                Tripod3Slot->SetContent(CreateTripodTierWidget(OwnerPlayer->SelectedSkillIndex, 2));
            }

            UE_LOG(LogTemp, Warning, TEXT("트라이포드 위젯 갱신 완료 - 선택된 스킬 인덱스: %d"), OwnerPlayer->SelectedSkillIndex);

            this->SetRenderOpacity(0.99f);
            this->SetRenderOpacity(1.0f);
        }
    }
}

TSharedRef<SWidget> STripodSystemWidget::CreateTripodTierWidget(int32 SkillIndex, int32 TierIndex)
{
    // 로그 추가: 현재 생성 중인 스킬 및 티어 정보 확인
    UE_LOG(LogTemp, Warning, TEXT("CreateTripodTierWidget 호출됨 - 스킬 인덱스: %d, 티어 인덱스: %d"), SkillIndex, TierIndex);

    // 유효성 검사 추가
    if (!OwnerPlayer.IsValid() || !OwnerPlayer->Skills.IsValidIndex(SkillIndex) ||
        !OwnerPlayer->Skills[SkillIndex].TripodTiers.IsValidIndex(TierIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("CreateTripodTierWidget 오류 - 잘못된 인덱스: 스킬(%d), 티어(%d)"), SkillIndex, TierIndex);
        return SNew(SBox)
            [SNew(STextBlock)
                 .Text(FText::FromString(TEXT("데이터가 없습니다")))
                 .ColorAndOpacity(FSlateColor(FLinearColor::Gray))];
    }

    // SkillIndex가 아니라 현재 선택된 스킬을 사용하도록 수정
    const int32 CurrentSkillIndex = OwnerPlayer->SelectedSkillIndex;

    // 현재 선택된 스킬 인덱스 검증
    if (!OwnerPlayer->Skills.IsValidIndex(CurrentSkillIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("CreateTripodTierWidget 오류 - 선택된 스킬 인덱스가 유효하지 않음: %d"), CurrentSkillIndex);
        return SNew(SBox)
            [SNew(STextBlock)
                 .Text(FText::FromString(TEXT("선택된 스킬이 유효하지 않습니다")))
                 .ColorAndOpacity(FSlateColor(FLinearColor::Red))];
    }

    UE_LOG(LogTemp, Warning, TEXT("선택된 스킬 인덱스: %d, 이름: %s"),
           CurrentSkillIndex, *OwnerPlayer->Skills[CurrentSkillIndex].SkillName);

    const FSkillData &Skill = OwnerPlayer->Skills[CurrentSkillIndex];
    const FTripodTier &Tier = Skill.TripodTiers[TierIndex];

    // 트라이포드 티어의 활성화 여부에 따른 색상 변화
    FSlateColor TierColor = Tier.bIsUnlocked ? FSlateColor(FLinearColor::White) : FSlateColor(FLinearColor::Gray);

    return SNew(SVerticalBox) + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("Tier %d"), TierIndex + 1))).ColorAndOpacity(TierColor)]
           // 트라이포드 효과들을 여기에 추가
           + SVerticalBox::Slot()
                 .AutoHeight()
                     [SNew(SHorizontalBox) + SHorizontalBox::Slot()[CreateTripodEffectWidget(CurrentSkillIndex, TierIndex, 0)] + SHorizontalBox::Slot()[CreateTripodEffectWidget(CurrentSkillIndex, TierIndex, 1)] + SHorizontalBox::Slot()[CreateTripodEffectWidget(CurrentSkillIndex, TierIndex, 2)]];
}

TSharedRef<SWidget> STripodSystemWidget::CreateTripodEffectWidget(int32 SkillIndex, int32 TierIndex, int32 EffectIndex)
{
    // 로그 추가: 현재 생성 중인 효과 위젯 정보 확인
    UE_LOG(LogTemp, Display, TEXT("CreateTripodEffectWidget 호출됨 - 스킬(%d), 티어(%d), 효과(%d)"),
           SkillIndex, TierIndex, EffectIndex);

    // 유효성 검사 강화
    if (!OwnerPlayer.IsValid() || !OwnerPlayer->Skills.IsValidIndex(SkillIndex) ||
        !OwnerPlayer->Skills[SkillIndex].TripodTiers.IsValidIndex(TierIndex) ||
        EffectIndex >= OwnerPlayer->Skills[SkillIndex].TripodTiers[TierIndex].TripodEffects.Num())
    {
        // 기본 더미 위젯 반환
        UE_LOG(LogTemp, Warning, TEXT("CreateTripodEffectWidget 오류 - 잘못된 인덱스"));
        return SNew(SBox)
            .WidthOverride(64)
            .HeightOverride(64)
                [SNew(SBorder)
                     .HAlign(HAlign_Center)
                     .VAlign(VAlign_Center)
                         [SNew(STextBlock)
                              .Text(FText::FromString(TEXT("---")))
                              .ColorAndOpacity(FSlateColor(FLinearColor::Gray))]];
    }

    const FSkillData &Skill = OwnerPlayer->Skills[SkillIndex];
    const FTripodTier &Tier = Skill.TripodTiers[TierIndex];

    // 해금되지 않은 경우 비활성화된 상태로 표시
    bool bIsEnabled = Tier.bIsUnlocked;

    // 선택된 효과인지 확인
    bool bIsSelected = (Tier.SelectedEffectIndex == EffectIndex);

    // 효과 색상 설정 (선택/활성화/비활성화 상태에 따라 다른 색상)
    FSlateColor EffectColor;
    if (!bIsEnabled)
    {
        EffectColor = FSlateColor(FLinearColor::Gray); // 비활성화
    }
    else if (bIsSelected)
    {
        EffectColor = FSlateColor(FLinearColor(0.2f, 0.8f, 1.0f, 1.0f)); // 선택됨 (밝은 파란색)
    }
    else
    {
        EffectColor = FSlateColor(FLinearColor::White); // 활성화됨
    }

    // TripodEffects 배열에 해당 인덱스가 있는지 확인
    if (!Tier.TripodEffects.IsValidIndex(EffectIndex))
    {
        // 비어있는 효과 표시
        return SNew(SButton)
            .IsEnabled(false)
                [SNew(SVerticalBox) + SVerticalBox::Slot()
                                          .AutoHeight()
                                              [SNew(STextBlock)
                                                   .Text(FText::FromString(TEXT("비어있음")))
                                                   .ColorAndOpacity(FSlateColor(FLinearColor::Gray))]];
    }

    const FTripodEffect &Effect = Tier.TripodEffects[EffectIndex];

    // 현재 선택된 실제 스킬 인덱스 가져오기
    int32 CurrentSkillIndex = OwnerPlayer->SelectedSkillIndex;

    return SNew(SButton)
        // 현재 선택된 스킬 인덱스(CurrentSkillIndex)를 사용하도록 수정
        .OnClicked(this, &STripodSystemWidget::OnTripodEffectSelected, CurrentSkillIndex, TierIndex, EffectIndex)
        .IsEnabled(bIsEnabled)
            [SNew(SBorder)
                 .BorderImage(FCoreStyle::Get().GetBrush("Border"))
                 .BorderBackgroundColor(bIsSelected ? FLinearColor(0.0f, 0.5f, 1.0f, 0.5f) : FLinearColor(0.0f, 0.0f, 0.0f, 0.0f))
                 .Padding(FMargin(4.0f))
                     [SNew(SVerticalBox) + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(Effect.EffectName)).ColorAndOpacity(EffectColor)] + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(Effect.Description)).ColorAndOpacity(EffectColor)]]];
}

FReply STripodSystemWidget::OnSkillSelected(int32 SkillIndex)
{
    if (OwnerPlayer.IsValid() && OwnerPlayer->Skills.IsValidIndex(SkillIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("스킬 선택: 인덱스 %d, 이름: %s"),
               SkillIndex, *OwnerPlayer->Skills[SkillIndex].SkillName);

        // 선택된 스킬 인덱스 설정
        OwnerPlayer->SelectedSkillIndex = SkillIndex;

        // UI 갱신
        RefreshWidget();
    }

    return FReply::Handled();
}

FReply STripodSystemWidget::OnTripodEffectSelected(int32 SkillIndex, int32 TierIndex, int32 EffectIndex)
{
    if (OwnerPlayer.IsValid())
    {
        // 현재 선택된 스킬 인덱스 사용
        int32 CurrentSkillIndex = OwnerPlayer->SelectedSkillIndex;

        UE_LOG(LogTemp, Warning, TEXT("트라이포드 효과 선택: 파라미터 스킬(%d), 실제 선택된 스킬(%d), 티어(%d), 효과(%d)"),
               SkillIndex, CurrentSkillIndex, TierIndex, EffectIndex);

        // 트라이포드 효과 선택 처리 - 현재 선택된 스킬 인덱스 사용
        OwnerPlayer->SelectTripod(CurrentSkillIndex, TierIndex, EffectIndex);

        // UI 갱신하여 선택된 효과를 시각적으로 표시
        RefreshWidget();
    }

    return FReply::Handled();
}

FReply STripodSystemWidget::OnLevelUpButtonClicked(int32 SkillIndex)
{
    if (OwnerPlayer.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("레벨업 버튼 클릭: 스킬 인덱스 %d"), SkillIndex);

        // 스킬 레벨업
        OwnerPlayer->LevelUpSkill(SkillIndex);

        // UI 갱신
        RefreshWidget();
    }

    return FReply::Handled();
}

FReply STripodSystemWidget::OnLevelDownButtonClicked(int32 SkillIndex)
{
    if (OwnerPlayer.IsValid() && OwnerPlayer->Skills.IsValidIndex(SkillIndex) &&
        OwnerPlayer->Skills[SkillIndex].SkillLevel > 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("레벨다운 버튼 클릭: 스킬 인덱스 %d"), SkillIndex);

        // 스킬 레벨다운
        OwnerPlayer->LevelDownSkill(SkillIndex);

        // UI 갱신
        RefreshWidget();
    }

    return FReply::Handled();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION