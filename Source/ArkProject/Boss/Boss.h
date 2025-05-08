// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Boss.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBossHitDelegate, AActor*, OtherActor, FName, BoneName);

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage)
	float LeftAttackDamage = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage)
	float RightAttackDamage = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage)
	float TailAttackDamage = 30.f;

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
	
	// 데미지 텍스트 액터 클래스
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class ADamageTextActor> DamageTextActorClass;

	// 충돌 처리용 컴포넌트 추가
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	class UBoxComponent* HitCollision;

	// 왼손 공격 처리용 컴포넌트 추가
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	class UBoxComponent* LeftHitCollision;

	// 오른손 공격 처리용 컴포넌트 추가
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	class UBoxComponent* RightHitCollision;

	// 꼬리 공격 처리용 컴포넌트 추가
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	class UBoxComponent* BackHitCollision;

	// 충돌 감지 이벤트 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBossHitDelegate OnLeftHandHit;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBossHitDelegate OnRightHandHit;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBossHitDelegate OnTailHit;

	// 충돌 이벤트 처리 함수
	UFUNCTION()
	void OnLeftHandOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRightHandOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTailOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 공격 활성화/비활성화 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ActivateLeftHandAttack(bool bActivate);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ActivateRightHandAttack(bool bActivate);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ActivateTailAttack(bool bActivate);

	// 데미지 적용 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyDamageToTarget(AActor* TargetActor, float DamageAmount);
};
