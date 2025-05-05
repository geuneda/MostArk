// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Boss.generated.h"

UCLASS()
class ARKPROJECT_API ABoss : public ACharacter
{
	GENERATED_BODY()

private:
	float CurrentHealth;
	
public:
	// Sets default values for this character's properties
	ABoss();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 데미지 처리 함수 오버라이드
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 1000000.f;

	// 현재 체력 getter
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	// 체력 비율 getter (0.0 ~ 1.0)
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetHealthPercentage() const { return CurrentHealth / MaxHealth; }

	// 데미지 받을 때 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Health")
	void OnDamaged(float Damage, AActor* DamageCauser);

	// 사망 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Health")
	void OnDeath();
	
	// 데미지 텍스트 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UDamageTextWidget> DamageTextWidgetClass;
};
