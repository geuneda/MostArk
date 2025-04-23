// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArkProject/ArkProjectCharacter.h"
#include "MostArkPlayer.generated.h"

// 트라이포드 효과 정의
USTRUCT(BlueprintType)
struct FTripodEffect
{
    GENERATED_BODY()

    // 효과 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tripod")
    FString EffectName;

    // 효과 설명
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tripod")
    FString Description;

    // 효과 아이콘
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tripod")
    UTexture2D* EffectIcon;

    // 효과 레벨
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tripod")
    int32 EffectLevel;

    // 기본 생성자
    FTripodEffect()
    {
        EffectName = TEXT("기본 효과");
        Description = TEXT("효과 설명");
        EffectIcon = nullptr;
        EffectLevel = 1;
    }
};

// 트라이포드 티어 정의
USTRUCT(BlueprintType)
struct FTripodTier
{
    GENERATED_BODY()

    // 티어 레벨 (1, 2, 3)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tripod")
    int32 TierLevel;

    // 해당 티어의 효과 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tripod")
    TArray<FTripodEffect> TripodEffects;

    // 선택된 효과 인덱스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tripod")
    int32 SelectedEffectIndex;

    // 티어 활성화 여부 (스킬 레벨에 따라)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tripod")
    bool bIsUnlocked;

    // 기본 생성자
    FTripodTier()
    {
        TierLevel = 1;
        SelectedEffectIndex = -1;
        bIsUnlocked = false;
    }
};

// 스킬 데이터 정의
USTRUCT(BlueprintType)
struct FSkillData
{
    GENERATED_BODY()

    // 스킬 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    FString SkillName;

    // 스킬 레벨
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    int32 SkillLevel;

    // 스킬 아이콘
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    UTexture2D* SkillIcon;

    // 스킬 쿨다운
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float Cooldown;

    // 스킬 데미지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float Damage;

    // 트라이포드 티어
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    TArray<FTripodTier> TripodTiers;

    // 기본 생성자
    FSkillData()
    {
        SkillName = TEXT("기본 스킬");
        SkillLevel = 1;
        SkillIcon = nullptr;
        Cooldown = 10.0f;
        Damage = 100.0f;

        // 기본적으로 3단계의 트라이포드 생성
        TripodTiers.SetNum(3);
        for (int32 i = 0; i < 3; i++)
        {
            TripodTiers[i].TierLevel = i + 1;
        }
    }
};

UCLASS()
class ARKPROJECT_API AMostArkPlayer : public AArkProjectCharacter
{
    GENERATED_BODY()

public:
    AMostArkPlayer();

    virtual void BeginPlay() override;

    virtual void Tick(float DeltaTime) override;

    // 입력 바인딩을 위한 함수
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // 스킬 사용 함수
    UFUNCTION(BlueprintCallable, Category = "Skill")
    void UseSkill(int32 SkillIndex);
    void ExecuteSkillEffect(int32 SkillIndex);

    // 트라이포드 선택 함수
    UFUNCTION(BlueprintCallable, Category = "Tripod")
    void SelectTripod(int32 SkillIndex, int32 TierIndex, int32 EffectIndex);

    // 스킬 레벨업 함수
    UFUNCTION(BlueprintCallable, Category = "Skill")
    void LevelUpSkill(int32 SkillIndex);

    // 트라이포드 해금 확인 함수
    UFUNCTION(BlueprintCallable, Category = "Tripod")
    void CheckTripodUnlock(int32 SkillIndex);

    // 스킬 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
    TArray<FSkillData> Skills;

    // 현재 선택된 스킬 인덱스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
    int32 SelectedSkillIndex;

    // 스킬 포인트
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
    int32 SkillPoints;

    // 트라이포드 UI 토글 함수
    UFUNCTION(BlueprintCallable, Category = "UI")
    void ToggleTripodSystemUI();

    /** Returns TopDownCameraComponent subobject **/
    FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
    /** Returns CameraBoom subobject **/
    FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

private:
    /** Top down camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* TopDownCameraComponent;

    /** Camera boom positioning the camera above the character */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* CameraBoom;

    // 트라이포드 효과 적용 함수
    void ApplyTripodEffects(int32 SkillIndex);
    void ApplyShockEffects(FSkillData& Skill, int32 TierLevel, const FString& EffectName);
    void ApplyBladeStormEffects(FSkillData& Skill, int32 TierLevel, const FString& EffectName);
    void ApplyWindBladeEffects(FSkillData& Skill, int32 TierLevel, const FString& EffectName);

    // 트라이포드 효과 설정 함수
    void SetupTripodEffects();
};