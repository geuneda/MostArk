#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "DamageTextActor.generated.h"

/**
 * 월드 공간에 데미지 텍스트를 표시하는 액터
 */
UCLASS()
class ARKPROJECT_API ADamageTextActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ADamageTextActor();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	// 텍스트 렌더 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UTextRenderComponent* TextRender;

	// 수명 타이머
	float LifeTime;
	
	// 최대 수명
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Text")
	float MaxLifeTime;
	
	// 상승 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Text")
	float RiseSpeed;
	
	// 크리티컬 여부
	bool bIsCritical;

public:
	// 데미지 텍스트 초기화
	void Initialize(float Damage, bool bInIsCritical);
	
	// 빌보드 효과 (항상 카메라를 향하도록)
	void FaceCamera();
};
