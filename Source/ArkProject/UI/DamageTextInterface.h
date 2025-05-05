#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DamageTextInterface.generated.h"

// 이 클래스는 반드시 UDamageTextInterface 앞에 선언되어야 합니다.
UINTERFACE(MinimalAPI, Blueprintable)
class UDamageTextInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 데미지 텍스트 위젯과 통신하기 위한 인터페이스
 */
class ARKPROJECT_API IDamageTextInterface
{
	GENERATED_BODY()

public:
	// 데미지 텍스트 초기화 함수
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Damage")
	void InitDamageText(float Damage, const FVector2D& ScreenPosition, bool bIsCritical);
};
