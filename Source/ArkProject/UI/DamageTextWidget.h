#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"
#include "DamageTextWidget.generated.h"

/**
 * 데미지 숫자를 표시하는 위젯
 */
UCLASS()
class ARKPROJECT_API UDamageTextWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	// 데미지 텍스트 위젯
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DamageText;
	
	// 페이드 아웃 애니메이션
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeOutAnimation;
	
	// 위젯 초기화 완료 후 호출
	virtual void NativeConstruct() override;
	
	// 틱마다 호출
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	// 월드 위치 기반 업데이트
	void UpdateWorldPosition();
	
private:
	// 애니메이션 타이머
	float AnimationTime;
	
	// 애니메이션 지속 시간
	float AnimationDuration;
	
	// 위젯 제거 예정 여부
	bool bPendingKill;
	
	// 시작 위치 (스크린 좌표)
	FVector2D StartPosition;
	
	// 이동 속도
	FVector2D MoveSpeed;
	
	// 월드 위치 추적 여부
	bool bTrackWorldLocation;
	
	// 추적 대상 월드 위치
	FVector WorldLocation;
	
	// 추적 대상 액터
	TWeakObjectPtr<AActor> TargetActor;
	
	// 추적 대상으로부터의 오프셋
	FVector LocationOffset;
	
public:
	UDamageTextWidget(const FObjectInitializer& ObjectInitializer);
	
	// 위젯 초기화
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void SetDamageText(float Damage);
	
	// 위젯 위치 설정 (스크린 좌표)
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void SetWidgetPosition(const FVector2D& Position);
	
	// 위젯 위치 설정 (월드 좌표)
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void SetWorldPosition(const FVector& InWorldLocation, AActor* InTargetActor = nullptr, const FVector& InOffset = FVector::ZeroVector);
	
	// 위젯 색상 설정 (크리티컬 등 표현)
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void SetTextColor(bool bIsCritical);
	
	// 애니메이션 재생
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void PlayDamageAnimation();
	
	// 데미지 텍스트 초기화 (모든 설정 한번에)
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void InitializeDamageText(float Damage, const FVector2D& Position, bool bIsCritical);
	
	// 데미지 텍스트 초기화 (월드 위치 기반)
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void InitializeDamageTextWorld(float Damage, const FVector& InWorldLocation, bool bIsCritical, AActor* InTargetActor = nullptr, const FVector& InOffset = FVector::ZeroVector);
};
