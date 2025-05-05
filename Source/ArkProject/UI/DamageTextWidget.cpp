#include "DamageTextWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/GameplayStatics.h"

UDamageTextWidget::UDamageTextWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , AnimationTime(0.0f)
    , AnimationDuration(2.0f)
    , bPendingKill(false)
    , StartPosition(FVector2D::ZeroVector)
    , MoveSpeed(FVector2D(0.0f, -50.0f)) // 초당 50 유닛 위로 이동
    , bTrackWorldLocation(false)
    , WorldLocation(FVector::ZeroVector)
    , TargetActor(nullptr)
    , LocationOffset(FVector::ZeroVector)
{
}

void UDamageTextWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 애니메이션이 있으면 지속 시간 가져오기
    if (FadeOutAnimation)
    {
        AnimationDuration = FadeOutAnimation->GetEndTime();
    }
}

void UDamageTextWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    // 애니메이션 진행 중이면 타이머 업데이트
    if (!bPendingKill)
    {
        AnimationTime += InDeltaTime;
        
        // 애니메이션 종료 체크
        if (AnimationTime >= AnimationDuration)
        {
            bPendingKill = true;
            RemoveFromParent();
            return;
        }
        
        // 월드 위치 추적 모드인 경우 위치 업데이트
        if (bTrackWorldLocation)
        {
            UpdateWorldPosition();
        }
        // 일반 모드인 경우 (애니메이션이 없을 때) 위치 업데이트
        else if (!FadeOutAnimation && DamageText)
        {
            // 현재 위치 계산 (시작 위치에서 위로 이동)
            FVector2D CurrentPosition = StartPosition + MoveSpeed * AnimationTime;
            
            // 위치 설정
            if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
            {
                CanvasSlot->SetPosition(CurrentPosition);
            }
            
            // 알파값 계산 (시간에 따라 페이드 아웃)
            float Alpha = 1.0f - FMath::Clamp(AnimationTime / AnimationDuration, 0.0f, 1.0f);
            
            // 텍스트 색상 업데이트 (deprecated 방식 수정)
            FLinearColor CurrentColor = DamageText->GetColorAndOpacity().GetSpecifiedColor();
            CurrentColor.A = Alpha;
            DamageText->SetColorAndOpacity(CurrentColor);
        }
    }
}

void UDamageTextWidget::UpdateWorldPosition()
{
    // 월드가 유효한지 확인
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // 플레이어 컨트롤러 가져오기
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC)
    {
        return;
    }
    
    // 추적할 위치 계산
    FVector LocationToTrack = WorldLocation;
    
    // 타겟 액터가 있으면 액터 위치 + 오프셋 사용
    if (TargetActor.IsValid())
    {
        LocationToTrack = TargetActor->GetActorLocation() + LocationOffset;
    }
    
    // 월드 위치를 스크린 위치로 변환
    FVector2D ScreenPosition;
    bool bResult = UGameplayStatics::ProjectWorldToScreen(
        PC,
        LocationToTrack,
        ScreenPosition,
        false
    );
    
    if (bResult)
    {
        // 애니메이션 효과 적용 (위로 올라가는 효과)
        ScreenPosition += FVector2D(0.0f, -50.0f) * AnimationTime;
        
        // 위치 설정
        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
        {
            CanvasSlot->SetPosition(ScreenPosition);
        }
        
        // 알파값 계산 (시간에 따라 페이드 아웃)
        if (DamageText && !FadeOutAnimation)
        {
            float Alpha = 1.0f - FMath::Clamp(AnimationTime / AnimationDuration, 0.0f, 1.0f);
            
            // 텍스트 색상 업데이트 (deprecated 방식 수정)
            FLinearColor CurrentColor = DamageText->GetColorAndOpacity().GetSpecifiedColor();
            CurrentColor.A = Alpha;
            DamageText->SetColorAndOpacity(CurrentColor);
        }
    }
}

void UDamageTextWidget::SetWidgetPosition(const FVector2D& Position)
{
    if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
    {
        CanvasSlot->SetPosition(Position);
        StartPosition = Position;
    }
    
    // 스크린 위치 모드로 설정
    bTrackWorldLocation = false;
}

void UDamageTextWidget::SetWorldPosition(const FVector& InWorldLocation, AActor* InTargetActor, const FVector& InOffset)
{
    // 월드 위치 저장
    WorldLocation = InWorldLocation;
    TargetActor = InTargetActor;
    LocationOffset = InOffset;
    
    // 월드 위치 추적 모드로 설정
    bTrackWorldLocation = true;
    
    // 초기 위치 설정
    UpdateWorldPosition();
}

void UDamageTextWidget::SetDamageText(float Damage)
{
    if (DamageText)
    {
        // 데미지 값을 텍스트로 변환 (소수점 없이)
        FText DamageString = FText::AsNumber(FMath::RoundToInt(Damage));
        DamageText->SetText(DamageString);
        
        // 데미지 크기에 따라 텍스트 크기 조정 (선택 사항)
        float TextScale = FMath::Clamp(1.0f + (Damage / 1000.0f), 1.0f, 1.5f);
        DamageText->SetRenderScale(FVector2D(TextScale, TextScale));
    }
}

void UDamageTextWidget::SetTextColor(bool bIsCritical)
{
    if (DamageText)
    {
        // 크리티컬이면 빨간색, 아니면 흰색
        FLinearColor TextColor = bIsCritical ? 
            FLinearColor(1.0f, 0.0f, 0.0f, 1.0f) : // 빨간색
            FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);  // 흰색
            
        DamageText->SetColorAndOpacity(TextColor);
    }
}

void UDamageTextWidget::PlayDamageAnimation()
{
    // 애니메이션 타이머 초기화
    AnimationTime = 0.0f;
    bPendingKill = false;
    
    // 애니메이션이 있으면 재생
    if (FadeOutAnimation)
    {
        PlayAnimation(FadeOutAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f);
    }
}

void UDamageTextWidget::InitializeDamageText(float Damage, const FVector2D& Position, bool bIsCritical)
{
    // 위치 설정 (스크린 좌표)
    SetWidgetPosition(Position);
    
    // 데미지 텍스트 설정
    SetDamageText(Damage);
    
    // 색상 설정 (크리티컬 여부에 따라)
    SetTextColor(bIsCritical);
    
    // 애니메이션 재생
    PlayDamageAnimation();
    
    // 약간의 랜덤 오프셋 추가 (여러 데미지 텍스트가 겹치지 않도록)
    if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
    {
        FVector2D RandomOffset(
            FMath::RandRange(-20.0f, 20.0f),
            FMath::RandRange(-20.0f, 20.0f)
        );
        FVector2D NewPosition = Position + RandomOffset;
        CanvasSlot->SetPosition(NewPosition);
        StartPosition = NewPosition;
    }
}

void UDamageTextWidget::InitializeDamageTextWorld(float Damage, const FVector& InWorldLocation, bool bIsCritical, AActor* InTargetActor, const FVector& InOffset)
{
    // 월드 위치 설정
    SetWorldPosition(InWorldLocation, InTargetActor, InOffset);
    
    // 데미지 텍스트 설정
    SetDamageText(Damage);
    
    // 색상 설정 (크리티컬 여부에 따라)
    SetTextColor(bIsCritical);
    
    // 애니메이션 재생
    PlayDamageAnimation();
}
