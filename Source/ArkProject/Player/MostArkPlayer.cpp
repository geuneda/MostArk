// Fill out your copyright notice in the Description page of Project Settings.

#include "MostArkPlayer.h"
#include "Components/InputComponent.h"
#include "../HUD/TripodSystemHUD.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"

AMostArkPlayer::AMostArkPlayer()
{
    PrimaryActorTick.bCanEverTick = true;

    // 기본값 설정
    SelectedSkillIndex = 0;
    SkillPoints = 20; // 스킬 포인트 초기값 설정

    // 예시 스킬 생성
    FSkillData Skill1;
    Skill1.SkillName = TEXT("충격파");
    Skills.Add(Skill1);

    FSkillData Skill2;
    Skill2.SkillName = TEXT("블레이드 스톰");
    Skills.Add(Skill2);

    FSkillData Skill3;
    Skill3.SkillName = TEXT("윈드 블레이드");
    Skills.Add(Skill3);

    // Set size for player capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
    GetCharacterMovement()->bConstrainToPlane = true;
    GetCharacterMovement()->bSnapToPlaneAtStart = true;

    // Create a camera boom...
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->SetUsingAbsoluteRotation(true);
    CameraBoom->TargetArmLength = 800.f;
    CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
    CameraBoom->bDoCollisionTest = false;

    // Create a camera...
    TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
    TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
}

void AMostArkPlayer::BeginPlay()
{
    Super::BeginPlay();
    
    // 예시 트라이포드 효과 설정
    SetupTripodEffects();
    
    // 초기 스킬 설정
    for (int32 i = 0; i < Skills.Num(); ++i)
    {
        CheckTripodUnlock(i);
    }
}

void AMostArkPlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AMostArkPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAction(TEXT("ToggleTripodUI"), IE_Pressed, this, &AMostArkPlayer::ToggleTripodSystemUI);
}

// 스킬 사용 함수
void AMostArkPlayer::UseSkill(int32 SkillIndex)
{
    if (Skills.IsValidIndex(SkillIndex))
    {
        // 스킬 사용 전 트라이포드 효과 적용
        ApplyTripodEffects(SkillIndex);

        // 여기에 실제 스킬 사용 로직을 구현
        FSkillData& Skill = Skills[SkillIndex];
        UE_LOG(LogTemp, Display, TEXT("스킬 사용: %s (데미지: %.1f, 쿨다운: %.1f)"), 
            *Skill.SkillName, Skill.Damage, Skill.Cooldown);
        
        // 스킬 효과 시뮬레이션 (실제로는 여기서 게임플레이 로직을 구현)
        ExecuteSkillEffect(SkillIndex);
    }
}

// 스킬 효과 실행 함수
void AMostArkPlayer::ExecuteSkillEffect(int32 SkillIndex)
{
    if (!Skills.IsValidIndex(SkillIndex))
        return;
        
    FSkillData& Skill = Skills[SkillIndex];
    
    // 스킬별 효과 구현
    if (Skill.SkillName.Equals(TEXT("충격파")))
    {
        // 기본 효과
        UE_LOG(LogTemp, Display, TEXT("충격파 발동! 전방에 %.1f 데미지의 충격을 발사합니다."), Skill.Damage);
        
        // 트라이포드에 따른 특수 효과
        // 1단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(0) && Skill.TripodTiers[0].bIsUnlocked && 
            Skill.TripodTiers[0].SelectedEffectIndex >= 0)
        {
            int32 Effect1Index = Skill.TripodTiers[0].SelectedEffectIndex;
            FString EffectName = Skill.TripodTiers[0].TripodEffects[Effect1Index].EffectName;
            
            if (EffectName.Equals(TEXT("강화된 충격")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [강화된 충격] 효과로 데미지가 30%% 증가했습니다!"));
            }
            else if (EffectName.Equals(TEXT("넓은 범위")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [넓은 범위] 효과로 공격 범위가 20%% 증가했습니다!"));
            }
            else if (EffectName.Equals(TEXT("빠른 충전")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [빠른 충전] 효과로 쿨다운이 15%% 감소했습니다!"));
            }
        }
        
        // 2단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(1) && Skill.TripodTiers[1].bIsUnlocked && 
            Skill.TripodTiers[1].SelectedEffectIndex >= 0)
        {
            int32 Effect2Index = Skill.TripodTiers[1].SelectedEffectIndex;
            FString EffectName = Skill.TripodTiers[1].TripodEffects[Effect2Index].EffectName;
            
            if (EffectName.Equals(TEXT("연속 충격")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [연속 충격] 효과로 충격파가 한 번 더 발동됩니다!"));
                UE_LOG(LogTemp, Display, TEXT("    두 번째 충격파 발동! 추가로 %.1f 데미지를 입힙니다."), Skill.Damage * 0.7f);
            }
            else if (EffectName.Equals(TEXT("충격 침투")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [충격 침투] 효과로 적의 방어력을 20%% 무시합니다!"));
            }
        }
        
        // 3단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(2) && Skill.TripodTiers[2].bIsUnlocked && 
            Skill.TripodTiers[2].SelectedEffectIndex >= 0)
        {
            int32 Effect3Index = Skill.TripodTiers[2].SelectedEffectIndex;
            FString EffectName = Skill.TripodTiers[2].TripodEffects[Effect3Index].EffectName;
            
            if (EffectName.Equals(TEXT("파괴적인 충격")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [파괴적인 충격] 효과로 적중 시 주변에 2차 폭발이 발생합니다!"));
                UE_LOG(LogTemp, Display, TEXT("    2차 폭발 발생! 주변 적들에게 %.1f 데미지를 입힙니다."), Skill.Damage * 0.5f);
            }
        }
    }
    else if (Skill.SkillName.Equals(TEXT("블레이드 스톰")))
    {
        // 기본 효과
        UE_LOG(LogTemp, Display, TEXT("블레이드 스톰 발동! 주변에 %.1f 데미지의 칼날 폭풍을 일으킵니다."), Skill.Damage);
        
        // 트라이포드에 따른 특수 효과
        // 1단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(0) && Skill.TripodTiers[0].bIsUnlocked && 
            Skill.TripodTiers[0].SelectedEffectIndex >= 0)
        {
            int32 Effect1Index = Skill.TripodTiers[0].SelectedEffectIndex;
            FString EffectName = Skill.TripodTiers[0].TripodEffects[Effect1Index].EffectName;
            
            if (EffectName.Equals(TEXT("강화된 블레이드")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [강화된 블레이드] 효과로 데미지가 25%% 증가했습니다!"));
            }
            else if (EffectName.Equals(TEXT("지속 시간 증가")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [지속 시간 증가] 효과로 블레이드 스톰의 지속 시간이 30%% 증가했습니다!"));
            }
            else if (EffectName.Equals(TEXT("무기 강화")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [무기 강화] 효과로 10초간 공격력이 15%% 증가합니다!"));
            }
        }
        
        // 2단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(1) && Skill.TripodTiers[1].bIsUnlocked && 
            Skill.TripodTiers[1].SelectedEffectIndex >= 0)
        {
            int32 Effect2Index = Skill.TripodTiers[1].SelectedEffectIndex;
            FString EffectName = Skill.TripodTiers[1].TripodEffects[Effect2Index].EffectName;
            
            if (EffectName.Equals(TEXT("화염 블레이드")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [화염 블레이드] 효과로 블레이드가 화염 속성을 가지며 3초간 화상 효과를 줍니다!"));
            }
            else if (EffectName.Equals(TEXT("회복의 칼날")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [회복의 칼날] 효과로 적중한 적 1명당 최대 체력의 1%%를 회복합니다!"));
            }
        }
        
        // 3단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(2) && Skill.TripodTiers[2].bIsUnlocked && 
            Skill.TripodTiers[2].SelectedEffectIndex >= 0)
        {
            int32 Effect3Index = Skill.TripodTiers[2].SelectedEffectIndex;
            FString EffectName = Skill.TripodTiers[2].TripodEffects[Effect3Index].EffectName;
            
            if (EffectName.Equals(TEXT("블레이드 춤")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [블레이드 춤] 효과로 블레이드 스톰 종료 후 전방으로 도약하며 강력한 일격을 가합니다!"));
                UE_LOG(LogTemp, Display, TEXT("    도약 공격으로 %.1f 데미지를 입힙니다."), Skill.Damage * 1.5f);
            }
            else if (EffectName.Equals(TEXT("죽음의 소용돌이")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [죽음의 소용돌이] 효과로 블레이드 스톰의 크기가 50%% 증가하고 중심부에서 더 강한 데미지를 입힙니다!"));
            }
        }
    }
    else if (Skill.SkillName.Equals(TEXT("윈드 블레이드")))
    {
        // 기본 효과
        UE_LOG(LogTemp, Display, TEXT("윈드 블레이드 발동! 전방으로 %.1f 데미지의 바람 칼날을 날립니다."), Skill.Damage);
        
        // 트라이포드에 따른 특수 효과
        // 1단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(0) && Skill.TripodTiers[0].bIsUnlocked && 
            Skill.TripodTiers[0].SelectedEffectIndex >= 0)
        {
            int32 Effect1Index = Skill.TripodTiers[0].SelectedEffectIndex;
            FString EffectName = Skill.TripodTiers[0].TripodEffects[Effect1Index].EffectName;
            
            if (EffectName.Equals(TEXT("관통 바람")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [관통 바람] 효과로 바람 칼날이 적을 관통합니다!"));
            }
            else if (EffectName.Equals(TEXT("빠른 발사")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [빠른 발사] 효과로 시전 속도가 30%% 증가했습니다!"));
            }
            else if (EffectName.Equals(TEXT("예리한 칼날")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [예리한 칼날] 효과로 크리티컬 확률이 15%% 증가했습니다!"));
            }
        }
        
        // 2단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(1) && Skill.TripodTiers[1].bIsUnlocked && 
            Skill.TripodTiers[1].SelectedEffectIndex >= 0)
        {
            int32 Effect2Index = Skill.TripodTiers[1].SelectedEffectIndex;
            FString EffectName = Skill.TripodTiers[1].TripodEffects[Effect2Index].EffectName;
            
            if (EffectName.Equals(TEXT("바람의 축복")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [바람의 축복] 효과로 5초간 이동 속도가 15%% 증가합니다!"));
            }
            else if (EffectName.Equals(TEXT("삼중 칼날")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [삼중 칼날] 효과로 바람 칼날이 3개로 분열됩니다!"));
                UE_LOG(LogTemp, Display, TEXT("    분열된 칼날 2개가 각각 %.1f 데미지를 추가로 입힙니다."), Skill.Damage * 0.5f);
            }
        }
        
        // 3단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(2) && Skill.TripodTiers[2].bIsUnlocked && 
            Skill.TripodTiers[2].SelectedEffectIndex >= 0)
        {
            int32 Effect3Index = Skill.TripodTiers[2].SelectedEffectIndex;
            FString EffectName = Skill.TripodTiers[2].TripodEffects[Effect3Index].EffectName;
            
            if (EffectName.Equals(TEXT("태풍의 힘")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [태풍의 힘] 효과로 바람 칼날이 적중 지점에 소형 토네이도를 생성합니다!"));
                UE_LOG(LogTemp, Display, TEXT("    토네이도가 3초간 유지되며 초당 %.1f 데미지를 입힙니다."), Skill.Damage * 0.3f);
            }
            else if (EffectName.Equals(TEXT("바람의 파괴자")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [바람의 파괴자] 효과로 데미지가 80%% 증가하지만 쿨다운이 50%% 증가합니다!"));
            }
        }
    }
}

// 트라이포드 선택 함수
void AMostArkPlayer::SelectTripod(int32 SkillIndex, int32 TierIndex, int32 EffectIndex)
{
    if (Skills.IsValidIndex(SkillIndex) && 
        Skills[SkillIndex].TripodTiers.IsValidIndex(TierIndex) &&
        Skills[SkillIndex].TripodTiers[TierIndex].TripodEffects.IsValidIndex(EffectIndex))
    {
        // 해당 티어가 잠금 해제되었는지 확인
        if (Skills[SkillIndex].TripodTiers[TierIndex].bIsUnlocked)
        {
            // 선택된 효과 설정
            Skills[SkillIndex].TripodTiers[TierIndex].SelectedEffectIndex = EffectIndex;
            
            UE_LOG(LogTemp, Display, TEXT("트라이포드 선택: 스킬(%s) 티어(%d) 효과(%s)"), 
                *Skills[SkillIndex].SkillName, 
                TierIndex + 1, 
                *Skills[SkillIndex].TripodTiers[TierIndex].TripodEffects[EffectIndex].EffectName);
            
            // 트라이포드 효과 적용
            ApplyTripodEffects(SkillIndex);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("트라이포드 티어가 잠겨있습니다. 스킬 레벨을 높이세요."));
        }
    }
}

// 스킬 레벨업 함수
void AMostArkPlayer::LevelUpSkill(int32 SkillIndex)
{
    if (Skills.IsValidIndex(SkillIndex) && SkillPoints > 0)
    {
        // 스킬 레벨 증가
        Skills[SkillIndex].SkillLevel++;
        SkillPoints--;
        
        UE_LOG(LogTemp, Display, TEXT("스킬 레벨업: %s (레벨 %d)"), 
            *Skills[SkillIndex].SkillName, Skills[SkillIndex].SkillLevel);
        
        // 트라이포드 잠금 해제 여부 확인
        CheckTripodUnlock(SkillIndex);
    }
}

// 스킬 레벨다운 함수
void AMostArkPlayer::LevelDownSkill(int32 SkillIndex)
{
    if (Skills.IsValidIndex(SkillIndex) && Skills[SkillIndex].SkillLevel > 1)
    {
        // 스킬 레벨 감소
        Skills[SkillIndex].SkillLevel--;
        SkillPoints++;

        UE_LOG(LogTemp, Display, TEXT("스킬 레벨감소: %s (레벨 %d)"),
            *Skills[SkillIndex].SkillName, Skills[SkillIndex].SkillLevel);

        CheckTripodUnlock(SkillIndex);
    }
}

// 트라이포드 해금 확인 함수
void AMostArkPlayer::CheckTripodUnlock(int32 SkillIndex)
{
    if (Skills.IsValidIndex(SkillIndex))
    {
        // 스킬 레벨에 따라 트라이포드 티어 해금
        int32 SkillLevel = Skills[SkillIndex].SkillLevel;
        
        // 1단계 트라이포드: 스킬 레벨 4 이상
        if (SkillLevel >= 1 && Skills[SkillIndex].TripodTiers.IsValidIndex(0))
        {
            Skills[SkillIndex].TripodTiers[0].bIsUnlocked = true;
        }
        
        // 2단계 트라이포드: 스킬 레벨 7 이상
        if (SkillLevel >= 7 && Skills[SkillIndex].TripodTiers.IsValidIndex(1))
        {
            Skills[SkillIndex].TripodTiers[1].bIsUnlocked = true;
        }
        
        // 3단계 트라이포드: 스킬 레벨 10 이상
        if (SkillLevel >= 10 && Skills[SkillIndex].TripodTiers.IsValidIndex(2))
        {
            Skills[SkillIndex].TripodTiers[2].bIsUnlocked = true;
        }
    }
}

// 트라이포드 잠금 확인 함수
void AMostArkPlayer::CheckTripodlock(int32 SkillIndex)
{
    if (Skills.IsValidIndex(SkillIndex))
    {
        FSkillData& Skill = Skills[SkillIndex];

        // 레벨에 따라 티어 자금 상태 설정
        // 1단계 : 4 | 2단게 : 7 | 3단계 : 10
        if (Skill.SkillLevel < 4 && Skill.TripodTiers.IsValidIndex(0))
        {
            Skill.TripodTiers[0].bIsUnlocked = false;
            Skill.TripodTiers[0].SelectedEffectIndex = -1; // 효과 초기화
        }

        if (Skill.SkillLevel < 7 && Skill.TripodTiers.IsValidIndex(1))
        {
            Skill.TripodTiers[1].bIsUnlocked = false;
            Skill.TripodTiers[1].SelectedEffectIndex = -1; // 효과 초기화
        }
        
        if (Skill.SkillLevel < 10 && Skill.TripodTiers.IsValidIndex(2))
        {
            Skill.TripodTiers[2].bIsUnlocked = false;
            Skill.TripodTiers[2].SelectedEffectIndex = -1; // 효과 초기화
        }


        UE_LOG(LogTemp, Display, TEXT("트라이포드 잠금 상태 업데이트: %s"), *Skill.SkillName);
    }
}

// 트라이포드 효과 적용 함수
void AMostArkPlayer::ApplyTripodEffects(int32 SkillIndex)
{
    if (Skills.IsValidIndex(SkillIndex))
    {
        // 스킬 원래 상태로 초기화
        FSkillData& Skill = Skills[SkillIndex];
        Skill.Damage = 100.0f + (Skill.SkillLevel * 10.0f); // 레벨당 기본 데미지 10씩 증가
        Skill.Cooldown = 10.0f;
        
        // 각 티어의 선택된 효과 적용
        for (int32 i = 0; i < Skill.TripodTiers.Num(); ++i)
        {
            FTripodTier& Tier = Skill.TripodTiers[i];
            
            if (Tier.bIsUnlocked && Tier.TripodEffects.IsValidIndex(Tier.SelectedEffectIndex))
            {
                FTripodEffect& Effect = Tier.TripodEffects[Tier.SelectedEffectIndex];
                
                // 스킬별로 다른 효과 적용
                if (Skill.SkillName.Equals(TEXT("충격파")))
                {
                    ApplyShockEffects(Skill, Tier.TierLevel, Effect.EffectName);
                }
                else if (Skill.SkillName.Equals(TEXT("블레이드 스톰")))
                {
                    ApplyBladeStormEffects(Skill, Tier.TierLevel, Effect.EffectName);
                }
                else if (Skill.SkillName.Equals(TEXT("윈드 블레이드")))
                {
                    ApplyWindBladeEffects(Skill, Tier.TierLevel, Effect.EffectName);
                }
                
                UE_LOG(LogTemp, Display, TEXT("트라이포드 효과 적용: %s"), *Effect.EffectName);
            }
        }
    }
}

// 충격파 트라이포드 효과 적용
void AMostArkPlayer::ApplyShockEffects(FSkillData& Skill, int32 TierLevel, const FString& EffectName)
{
    if (TierLevel == 1) // 1단계 트라이포드
    {
        if (EffectName.Equals(TEXT("강화된 충격")))
        {
            Skill.Damage *= 1.3f; // 데미지 30% 증가
        }
        else if (EffectName.Equals(TEXT("넓은 범위")))
        {
            // 범위 증가 효과는 ExecuteSkillEffect에서 구현
        }
        else if (EffectName.Equals(TEXT("빠른 충전")))
        {
            Skill.Cooldown *= 0.85f; // 쿨다운 15% 감소
        }
    }
    else if (TierLevel == 2) // 2단계 트라이포드
    {
        if (EffectName.Equals(TEXT("연속 충격")))
        {
            Skill.Damage *= 1.7f; // 1회차 100% + 2회차 70%
        }
        else if (EffectName.Equals(TEXT("충격 침투")))
        {
            // 방어력 무시는 ExecuteSkillEffect에서 구현
        }
    }
    else if (TierLevel == 3) // 3단계 트라이포드
    {
        if (EffectName.Equals(TEXT("파괴적인 충격")))
        {
            Skill.Damage *= 1.5f; // 2차 폭발을 포함한 총 데미지
        }
    }
}

// 블레이드 스톰 트라이포드 효과 적용
void AMostArkPlayer::ApplyBladeStormEffects(FSkillData& Skill, int32 TierLevel, const FString& EffectName)
{
    if (TierLevel == 1) // 1단계 트라이포드
    {
        if (EffectName.Equals(TEXT("강화된 블레이드")))
        {
            Skill.Damage *= 1.25f; // 데미지 25% 증가
        }
        else if (EffectName.Equals(TEXT("지속 시간 증가")))
        {
            // 지속 시간 증가는 ExecuteSkillEffect에서 구현
        }
        else if (EffectName.Equals(TEXT("무기 강화")))
        {
            // 사용 후 버프는 ExecuteSkillEffect에서 구현
        }
    }
    else if (TierLevel == 2) // 2단계 트라이포드
    {
        if (EffectName.Equals(TEXT("화염 블레이드")))
        {
            Skill.Damage *= 1.1f; // 화상 효과로 인한 10% 추가 데미지
        }
        else if (EffectName.Equals(TEXT("회복의 칼날")))
        {
            // 회복 효과는 ExecuteSkillEffect에서 구현
        }
    }
    else if (TierLevel == 3) // 3단계 트라이포드
    {
        if (EffectName.Equals(TEXT("블레이드 춤")))
        {
            Skill.Damage *= 1.5f; // 도약 일격을 포함한 총 데미지
        }
        else if (EffectName.Equals(TEXT("죽음의 소용돌이")))
        {
            Skill.Damage *= 1.3f; // 범위 증가와 중심부 강화 효과
        }
    }
}

// 윈드 블레이드 트라이포드 효과 적용
void AMostArkPlayer::ApplyWindBladeEffects(FSkillData& Skill, int32 TierLevel, const FString& EffectName)
{
    if (TierLevel == 1) // 1단계 트라이포드
    {
        if (EffectName.Equals(TEXT("관통 바람")))
        {
            Skill.Damage *= 1.15f; // 관통으로 인한 15% 추가 데미지
        }
        else if (EffectName.Equals(TEXT("빠른 발사")))
        {
            // 시전 속도 증가는 ExecuteSkillEffect에서 구현
        }
        else if (EffectName.Equals(TEXT("예리한 칼날")))
        {
            Skill.Damage *= 1.1f; // 크리티컬 확률 증가로 인한 평균 데미지 증가
        }
    }
    else if (TierLevel == 2) // 2단계 트라이포드
    {
        if (EffectName.Equals(TEXT("바람의 축복")))
        {
            // 이동 속도 버프는 ExecuteSkillEffect에서 구현
        }
        else if (EffectName.Equals(TEXT("삼중 칼날")))
        {
            Skill.Damage *= 2.0f; // 원래 데미지 100% + 분열 칼날 2개 각 50%
        }
    }
    else if (TierLevel == 3) // 3단계 트라이포드
    {
        if (EffectName.Equals(TEXT("태풍의 힘")))
        {
            Skill.Damage *= 1.9f; // 토네이도 지속 데미지 90% 추가
        }
        else if (EffectName.Equals(TEXT("바람의 파괴자")))
        {
            Skill.Damage *= 1.8f; // 데미지 80% 증가
            Skill.Cooldown *= 1.5f; // 쿨다운 50% 증가
        }
    }
}

// 트라이포드 UI 토글 함수
void AMostArkPlayer::ToggleTripodSystemUI()
{
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (ATripodSystemHUD* TripodHUD = Cast<ATripodSystemHUD>(PC->GetHUD()))
        {
            TripodHUD->ToggleTripodSystemUI();
        }
    }
}

// 트라이포드 효과 설정 함수 추가
void AMostArkPlayer::SetupTripodEffects()
{
    // 스킬 1: 충격파
    if (Skills.IsValidIndex(0))
    {
        // 1단계 트라이포드
        if (Skills[0].TripodTiers.IsValidIndex(0))
        {
            FTripodTier& Tier1 = Skills[0].TripodTiers[0];
            
            // 효과 1: 강화된 충격
            FTripodEffect Effect1;
            Effect1.EffectName = TEXT("강화된 충격");
            Effect1.Description = TEXT("충격파의 데미지가 30% 증가합니다.");
            Tier1.TripodEffects.Add(Effect1);
            
            // 효과 2: 넓은 범위
            FTripodEffect Effect2;
            Effect2.EffectName = TEXT("넓은 범위");
            Effect2.Description = TEXT("충격파의 범위가 20% 증가합니다.");
            Tier1.TripodEffects.Add(Effect2);
            
            // 효과 3: 빠른 충전
            FTripodEffect Effect3;
            Effect3.EffectName = TEXT("빠른 충전");
            Effect3.Description = TEXT("충격파의 쿨다운이 15% 감소합니다.");
            Tier1.TripodEffects.Add(Effect3);
        }
        
        // 2단계 트라이포드
        if (Skills[0].TripodTiers.IsValidIndex(1))
        {
            FTripodTier& Tier2 = Skills[0].TripodTiers[1];
            
            // 효과 1: 연속 충격
            FTripodEffect Effect1;
            Effect1.EffectName = TEXT("연속 충격");
            Effect1.Description = TEXT("충격파가 2회 연속 발동됩니다.");
            Tier2.TripodEffects.Add(Effect1);
            
            // 효과 2: 충격 침투
            FTripodEffect Effect2;
            Effect2.EffectName = TEXT("충격 침투");
            Effect2.Description = TEXT("적의 방어력을 20% 무시합니다.");
            Tier2.TripodEffects.Add(Effect2);
        }
        
        // 3단계 트라이포드
        if (Skills[0].TripodTiers.IsValidIndex(2))
        {
            FTripodTier& Tier3 = Skills[0].TripodTiers[2];
            
            // 효과 1: 파괴적인 충격
            FTripodEffect Effect1;
            Effect1.EffectName = TEXT("파괴적인 충격");
            Effect1.Description = TEXT("충격파가 적중하면 주변에 2차 폭발이 발생합니다.");
            Tier3.TripodEffects.Add(Effect1);
        }
    }
    
    // 스킬 2: 블레이드 스톰
    if (Skills.IsValidIndex(1))
    {
        // 1단계 트라이포드
        if (Skills[1].TripodTiers.IsValidIndex(0))
        {
            FTripodTier& Tier1 = Skills[1].TripodTiers[0];
            
            // 효과 1: 강화된 블레이드
            FTripodEffect Effect1;
            Effect1.EffectName = TEXT("강화된 블레이드");
            Effect1.Description = TEXT("블레이드 스톰의 데미지가 25% 증가합니다.");
            Tier1.TripodEffects.Add(Effect1);
            
            // 효과 2: 지속 시간 증가
            FTripodEffect Effect2;
            Effect2.EffectName = TEXT("지속 시간 증가");
            Effect2.Description = TEXT("블레이드 스톰의 지속 시간이 30% 증가합니다.");
            Tier1.TripodEffects.Add(Effect2);
            
            // 효과 3: 무기 강화
            FTripodEffect Effect3;
            Effect3.EffectName = TEXT("무기 강화");
            Effect3.Description = TEXT("블레이드 스톰 사용 후 10초간 공격력이 15% 증가합니다.");
            Tier1.TripodEffects.Add(Effect3);
        }

        // 2단계 트라이포드
        if (Skills[1].TripodTiers.IsValidIndex(1))
        {
            FTripodTier& Tier2 = Skills[1].TripodTiers[1];
            
            // 효과 1: 화염 블레이드
            FTripodEffect Effect1;
            Effect1.EffectName = TEXT("화염 블레이드");
            Effect1.Description = TEXT("블레이드가 화염 속성을 가지며 3초간 화상 효과를 줍니다.");
            Tier2.TripodEffects.Add(Effect1);
            
            // 효과 2: 회복의 칼날
            FTripodEffect Effect2;
            Effect2.EffectName = TEXT("회복의 칼날");
            Effect2.Description = TEXT("적중한 적 1명당 최대 체력의 1%를 회복합니다.");
            Tier2.TripodEffects.Add(Effect2);
        }
        
        // 3단계 트라이포드
        if (Skills[1].TripodTiers.IsValidIndex(2))
        {
            FTripodTier& Tier3 = Skills[1].TripodTiers[2];
            
            // 효과 1: 블레이드 춤
            FTripodEffect Effect1;
            Effect1.EffectName = TEXT("블레이드 춤");
            Effect1.Description = TEXT("블레이드 스톰 종료 후 전방으로 도약하며 강력한 일격을 가합니다.");
            Tier3.TripodEffects.Add(Effect1);
            
            // 효과 2: 죽음의 소용돌이
            FTripodEffect Effect2;
            Effect2.EffectName = TEXT("죽음의 소용돌이");
            Effect2.Description = TEXT("블레이드 스톰의 크기가 50% 증가하고 중심부에서 더 강한 데미지를 입힙니다.");
            Tier3.TripodEffects.Add(Effect2);
        }
    }
    
    // 스킬 3: 윈드 블레이드
    if (Skills.IsValidIndex(2))
    {
        // 1단계 트라이포드
        if (Skills[2].TripodTiers.IsValidIndex(0))
        {
            FTripodTier& Tier1 = Skills[2].TripodTiers[0];
            
            // 효과 1: 관통 바람
            FTripodEffect Effect1;
            Effect1.EffectName = TEXT("관통 바람");
            Effect1.Description = TEXT("바람 칼날이 적을 관통합니다.");
            Tier1.TripodEffects.Add(Effect1);
            
            // 효과 2: 빠른 발사
            FTripodEffect Effect2;
            Effect2.EffectName = TEXT("빠른 발사");
            Effect2.Description = TEXT("시전 속도가 30% 증가합니다.");
            Tier1.TripodEffects.Add(Effect2);
            
            // 효과 3: 예리한 칼날
            FTripodEffect Effect3;
            Effect3.EffectName = TEXT("예리한 칼날");
            Effect3.Description = TEXT("크리티컬 확률이 15% 증가합니다.");
            Tier1.TripodEffects.Add(Effect3);
        }
        
        // 2단계 트라이포드
        if (Skills[2].TripodTiers.IsValidIndex(1))
        {
            FTripodTier& Tier2 = Skills[2].TripodTiers[1];
            
            // 효과 1: 바람의 축복
            FTripodEffect Effect1;
            Effect1.EffectName = TEXT("바람의 축복");
            Effect1.Description = TEXT("5초간 이동 속도가 15% 증가합니다.");
            Tier2.TripodEffects.Add(Effect1);
            
            // 효과 2: 삼중 칼날
            FTripodEffect Effect2;
            Effect2.EffectName = TEXT("삼중 칼날");
            Effect2.Description = TEXT("바람 칼날이 3개로 분열됩니다.");
            Tier2.TripodEffects.Add(Effect2);
        }
        
        // 3단계 트라이포드
        if (Skills[2].TripodTiers.IsValidIndex(2))
        {
            FTripodTier& Tier3 = Skills[2].TripodTiers[2];
            
            // 효과 1: 태풍의 힘
            FTripodEffect Effect1;
            Effect1.EffectName = TEXT("태풍의 힘");
            Effect1.Description = TEXT("바람 칼날이 적중 지점에 소형 토네이도를 생성합니다.");
            Tier3.TripodEffects.Add(Effect1);
            
            // 효과 2: 바람의 파괴자
            FTripodEffect Effect2;
            Effect2.EffectName = TEXT("바람의 파괴자");
            Effect2.Description = TEXT("데미지가 80% 증가하지만 쿨다운이 50% 증가합니다.");
            Tier3.TripodEffects.Add(Effect2);
        }
    }
}