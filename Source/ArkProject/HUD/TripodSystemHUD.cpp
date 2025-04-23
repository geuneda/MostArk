// Fill out your copyright notice in the Description page of Project Settings.

#include "TripodSystemHUD.h"
#include "../Player/MostArkPlayer.h"
#include "Engine/GameViewportClient.h"
#include "Widgets/SViewport.h"

ATripodSystemHUD::ATripodSystemHUD()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // UI는 기본적으로 숨겨져 있음
    bShowTripodSystemUI = false;
}

void ATripodSystemHUD::BeginPlay()
{
    Super::BeginPlay();
    
    // 트라이포드 시스템 UI 초기화
    InitializeTripodSystemUI();
}

void ATripodSystemHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    
    // UI 해제
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->RemoveViewportWidgetContent(TripodSystemViewport.ToSharedRef());
    }
}

void ATripodSystemHUD::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void ATripodSystemHUD::ToggleTripodSystemUI()
{
    if (bShowTripodSystemUI)
    {
        HideTripodSystemUI();
    }
    else
    {
        ShowTripodSystemUI();
    }
    
    bShowTripodSystemUI = !bShowTripodSystemUI;
}

void ATripodSystemHUD::InitializeTripodSystemUI()
{
    // 플레이어 참조 가져오기
    AMostArkPlayer* MostArkPlayer = Cast<AMostArkPlayer>(GetOwningPlayerController()->GetPawn());
    
    if (MostArkPlayer && GEngine && GEngine->GameViewport)
    {
        // 트라이포드 시스템 위젯 생성
        TripodSystemWidget = SNew(STripodSystemWidget)
            .OwnerPlayer(MostArkPlayer);
        
        // 뷰포트에 추가
        TripodSystemViewport = SNew(SViewport)
            .RenderDirectlyToWindow(false)
            .IsEnabled(true)
            .EnableBlending(true)
            .EnableGammaCorrection(false)
            [
                SAssignNew(TripodSystemWidget, STripodSystemWidget)
                .OwnerPlayer(MostArkPlayer)
            ];
        
        // 초기에는 UI 숨기기
        GEngine->GameViewport->AddViewportWidgetContent(TripodSystemViewport.ToSharedRef(), 10);
        HideTripodSystemUI();
    }
}

void ATripodSystemHUD::ShowTripodSystemUI()
{
    if (TripodSystemViewport.IsValid())
    {
        TripodSystemViewport->SetVisibility(EVisibility::Visible);
    }
}

void ATripodSystemHUD::HideTripodSystemUI()
{
    if (TripodSystemViewport.IsValid())
    {
        TripodSystemViewport->SetVisibility(EVisibility::Hidden);
    }
}