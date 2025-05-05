// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss.h"

#include "ArkProject/UI/DamageTextWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "Blueprint/UserWidget.h"

// Sets default values
ABoss::ABoss()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABoss::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	
	// 충돌 프로파일 설정 (플레이어 공격과 투사체를 받을 수 있도록)
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Boss"));
}

// Called every frame
void ABoss::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABoss::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

// 데미지 처리 함수 구현
float ABoss::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	// 데미지 적용
	CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);
	
	// 데미지 로그 출력
	UE_LOG(LogTemp, Warning, TEXT("Boss - 받은 데미지 : %.2f DamageCauser %s. 현재 체력 : %.2f"), 
		ActualDamage, 
		DamageCauser ? *DamageCauser->GetName() : TEXT("Unknown"), 
		CurrentHealth);
	
	// 데미지 텍스트 표시
	if (ActualDamage > 0.0f && DamageTextWidgetClass)
	{
		// 플레이어 컨트롤러 가져오기
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PC)
		{
			// 데미지 위치 (머리 위쪽으로 약간 올림)
			FVector DamageLocation = this->GetActorLocation() + FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 1.5f);
			
			// 월드 위치를 스크린 위치로 변환
			FVector2D ScreenPosition;
			bool bResult = UGameplayStatics::ProjectWorldToScreen(
				PC,
				DamageLocation,
				ScreenPosition,
				false
			);
			
			if (bResult)
			{
				// 데미지 텍스트 위젯 생성
				UDamageTextWidget* DamageWidget = CreateWidget<UDamageTextWidget>(PC, DamageTextWidgetClass);
				if (DamageWidget)
				{
					// 뷰포트에 추가
					DamageWidget->AddToViewport();
					
					// 크리티컬 여부 (예시: 100 이상의 데미지는 크리티컬로 표시)
					bool bIsCritical = ActualDamage >= 100.0f;
					
					// 월드 위치 기반 데미지 텍스트 초기화 (보스 위치 추적)
					// 오프셋을 사용하여 보스 머리 위에 표시
					FVector Offset = FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 1.5f);
					DamageWidget->InitializeDamageTextWorld(ActualDamage, GetActorLocation(), bIsCritical, this, Offset);
				}
			}
		}
	}
	
	// 데미지 이벤트 호출
	OnDamaged(ActualDamage, DamageCauser);
	
	// 체력이 0이 되면 사망 처리
	if (CurrentHealth <= 0.0f)
	{
		// 사망 이벤트 호출
		OnDeath();
	}
	
	return ActualDamage;
}
