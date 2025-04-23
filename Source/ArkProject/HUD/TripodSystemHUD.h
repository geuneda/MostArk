// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "../Widget/STripodSystemWidget.h"
#include "TripodSystemHUD.generated.h"

/**
 * 트라이포드 시스템을 표시하는 HUD 클래스
 */
UCLASS()
class ARKPROJECT_API ATripodSystemHUD : public AHUD
{
    GENERATED_BODY()

public:
    ATripodSystemHUD();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;

    // 트라이포드 시스템 UI 토글
    UFUNCTION(BlueprintCallable, Category = "UI")
    void ToggleTripodSystemUI();

private:
    // Slate 뷰포트
    TSharedPtr<SViewport> TripodSystemViewport;
    
    // 트라이포드 시스템 위젯
    TSharedPtr<STripodSystemWidget> TripodSystemWidget;
    
    // UI 표시 여부
    bool bShowTripodSystemUI;
    
    // UI 초기화
    void InitializeTripodSystemUI();
    
    // UI 표시
    void ShowTripodSystemUI();
    
    // UI 숨기기
    void HideTripodSystemUI();
};