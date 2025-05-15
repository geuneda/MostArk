#include "DamageTextActor.h"
#include "Kismet/GameplayStatics.h"

ADamageTextActor::ADamageTextActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	
	// 텍스트 렌더 컴포넌트 생성
	TextRender = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextRender"));
	TextRender->SetupAttachment(RootComponent);
	TextRender->SetHorizontalAlignment(EHTA_Center);
	TextRender->SetVerticalAlignment(EVRTA_TextCenter);
	TextRender->SetTextRenderColor(FColor::White);
	TextRender->SetWorldSize(40.f); // 텍스트 크기
	
	// 기본 값 설정
	MaxLifeTime = 2.0f;
	RiseSpeed = 100.0f;
	LifeTime = 0.0f;
	bIsCritical = false;
}

// 게임 시작 시 호출
void ADamageTextActor::BeginPlay()
{
	Super::BeginPlay();
	
	// 카메라를 향하도록 설정
	FaceCamera();
}

// 매 프레임 호출
void ADamageTextActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// 수명 업데이트
	LifeTime += DeltaTime;
	
	// 수명이 다하면 제거
	if (LifeTime >= MaxLifeTime)
	{
		Destroy();
		return;
	}
	
	// 위로 상승
	FVector NewLocation = GetActorLocation();
	NewLocation.Z += RiseSpeed * DeltaTime;
	SetActorLocation(NewLocation);
	
	// 페이드 아웃 효과
	float Alpha = 1.0f - (LifeTime / MaxLifeTime);
	FColor CurrentColor = TextRender->TextRenderColor;
	CurrentColor.A = FMath::Clamp(Alpha * 255.0f, 0.0f, 255.0f);
	TextRender->SetTextRenderColor(CurrentColor);
	
	// 카메라를 향하도록 업데이트
	FaceCamera();
}

// 데미지 텍스트 초기화
void ADamageTextActor::Initialize(float Damage, bool bInIsCritical)
{
	// 데미지 값 설정
	FString DamageString = FString::FromInt(FMath::RoundToInt(Damage));
	TextRender->SetText(FText::FromString(DamageString));
	
	// 크리티컬 여부 저장
	bIsCritical = bInIsCritical;
	
	// 크리티컬이면 노란색, 아니면 흰색
	FColor TextColor = bIsCritical ? FColor::Yellow : FColor::White;
	TextRender->SetTextRenderColor(TextColor);
	
	// 크리티컬이면 텍스트 크기 증가
	float TextSize = bIsCritical ? 80.0f : 40.0f;
	TextRender->SetWorldSize(TextSize);
	
	// 약간의 랜덤 오프셋 추가 (여러 데미지 텍스트가 겹치지 않도록)
	FVector RandomOffset(
		FMath::RandRange(-20.0f, 20.0f),
		FMath::RandRange(-20.0f, 20.0f),
		0.0f
	);
	SetActorLocation(GetActorLocation() + RandomOffset);
}

// 빌보드 효과
void ADamageTextActor::FaceCamera()
{
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
		{
			FVector CameraLocation;
			FRotator CameraRotation;
			PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
			
			// 카메라를 향하는 회전 계산
			FRotator NewRotation = (CameraLocation - GetActorLocation()).Rotation();
			SetActorRotation(NewRotation);
		}
	}
}
