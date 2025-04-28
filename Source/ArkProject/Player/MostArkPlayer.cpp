// Fill out your copyright notice in the Description page of Project Settings.

#include "MostArkPlayer.h"
#include "Components/InputComponent.h"
#include "../HUD/TripodSystemHUD.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

AMostArkPlayer::AMostArkPlayer()
{
    PrimaryActorTick.bCanEverTick = true;

    WeaponComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponComp"));
    WeaponComp->SetupAttachment(GetMesh(), TEXT("WeaponSocket"));

    ConstructorHelpers::FObjectFinder<UStaticMesh> tempWeaponMesh(TEXT("/Script/Engine.StaticMesh'/Game/Assassin/Mesh/Separated_Mesh/Weapon/SM_sword_Sword.SM_sword_Sword'"));
    if (tempWeaponMesh.Succeeded())
    {
        WeaponComp->SetStaticMesh(tempWeaponMesh.Object);
    }

    GetCharacterMovement()->MaxAcceleration = 10000.f;
    GetCharacterMovement()->BrakingFrictionFactor = 0.f;
    FSkillData ShockSkill;
    ShockSkill.SkillName = TEXT("충격파");
    ShockSkill.SkillLevel = 1;
    Skills.Add(ShockSkill);

    FSkillData BladeStormSkill;
    BladeStormSkill.SkillName = TEXT("블레이드 스톰");
    BladeStormSkill.SkillLevel = 1;
    Skills.Add(BladeStormSkill);

    FSkillData WindBladeSkill;
    WindBladeSkill.SkillName = TEXT("윈드 블레이드");
    WindBladeSkill.SkillLevel = 1;
    Skills.Add(WindBladeSkill);

    // 기본 선택 스킬 설정
    SelectedSkillIndex = 0;

    // 초기 스킬 포인트 설정
    SkillPoints = 30;

    // 트라이포드 효과 초기화 (필수!)
    for (int32 SkillIndex = 0; SkillIndex < Skills.Num(); ++SkillIndex)
    {
        FSkillData &Skill = Skills[SkillIndex];

        // 각 티어별로 트라이포드 효과 초기화
        for (int32 TierIndex = 0; TierIndex < Skill.TripodTiers.Num(); ++TierIndex)
        {
            FTripodTier &Tier = Skill.TripodTiers[TierIndex];
            Tier.TripodEffects.SetNum(3); // 각 티어마다 3개의 효과 추가
        }
    }

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

    // 트라이포드 효과 설정 - 반드시 스킬 초기화 후에 호출
    SetupTripodEffects();

    // 초기 트라이포드 해금 상태 설정
    for (int32 i = 0; i < Skills.Num(); ++i)
    {
        CheckTripodUnlock(i);
    }

    // 쿨다운 관련 배열 초기화
    SkillCooldownTimers.SetNum(Skills.Num());
    bSkillAvailable.Init(true, Skills.Num());

    // 특수 효과 변수 초기화
    bHasSpeedBuff = false;
    OriginalMovementSpeed = GetCharacterMovement()->MaxWalkSpeed;
    bHasAttackBuff = false;
    OriginalDamageMultiplier = 1.0f;
    BurnEffectCount = 0;

    UE_LOG(LogTemp, Warning, TEXT("MostArkPlayer BeginPlay 완료"));
}

void AMostArkPlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AMostArkPlayer::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAction(TEXT("ToggleTripodUI"), IE_Pressed, this, &AMostArkPlayer::ToggleTripodSystemUI);

    PlayerInputComponent->BindAction(TEXT("UseSkill1"), IE_Pressed, this, &AMostArkPlayer::UseSkill1);
    PlayerInputComponent->BindAction(TEXT("UseSkill2"), IE_Pressed, this, &AMostArkPlayer::UseSkill2);
    PlayerInputComponent->BindAction(TEXT("UseSkill3"), IE_Pressed, this, &AMostArkPlayer::UseSkill3);
}

// 스킬 사용 함수
void AMostArkPlayer::UseSkill(int32 SkillIndex)
{
    if (Skills.IsValidIndex(SkillIndex))
    {
        // 스킬 사용 전 트라이포드 효과 적용
        ApplyTripodEffects(SkillIndex);

        // 여기에 실제 스킬 사용 로직을 구현
        FSkillData &Skill = Skills[SkillIndex];
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

    FSkillData &Skill = Skills[SkillIndex];

    // 스킬별 효과 구현
    if (Skill.SkillName.Equals(TEXT("충격파")))
    {
        // 기본 효과
        UE_LOG(LogTemp, Display, TEXT("충격파 발동! 전방에 %.1f 데미지의 충격을 발사합니다."), Skill.Damage);

        // 파티클 이펙트 생성
        if (ShockWaveEffect)
        {
            FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 100.f;
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ShockWaveEffect, SpawnLocation, GetActorRotation());
        }

        // 트라이포드에 따른 특수 효과 - 티어별로 선택된 효과만 적용
        // 1단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(0) && Skill.TripodTiers[0].bIsUnlocked &&
            Skill.TripodTiers[0].SelectedEffectIndex != -1 &&
            Skill.TripodTiers[0].TripodEffects.IsValidIndex(Skill.TripodTiers[0].SelectedEffectIndex))
        {
            int32 Effect1Index = Skill.TripodTiers[0].SelectedEffectIndex;
            FString EffectName = Skill.TripodTiers[0].TripodEffects[Effect1Index].EffectName;

            if (EffectName.Equals(TEXT("강화된 충격")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [강화된 충격] 효과로 데미지가 30%% 증가했습니다!"));
            }
            else if (EffectName.Equals(TEXT("넓은 영역")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [넓은 영역] 효과로 공격 범위가 40%% 증가했습니다!"));

                // 넓은 범위 효과 시각화
                if (ShockWaveEffect)
                {
                    FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 100.f;
                    FVector Scale(1.4f, 1.4f, 1.4f); // 범위 40% 증가
                    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ShockWaveEffect, SpawnLocation, GetActorRotation(), Scale);
                }
            }
            else if (EffectName.Equals(TEXT("빠른 충전")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [빠른 충전] 효과로 쿨다운이 15%% 감소했습니다!"));
            }
        }

        // 2단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(1) && Skill.TripodTiers[1].bIsUnlocked &&
            Skill.TripodTiers[1].SelectedEffectIndex != -1 &&
            Skill.TripodTiers[1].TripodEffects.IsValidIndex(Skill.TripodTiers[1].SelectedEffectIndex))
        {
            int32 Effect2Index = Skill.TripodTiers[1].SelectedEffectIndex;
            FString EffectName = Skill.TripodTiers[1].TripodEffects[Effect2Index].EffectName;

            if (EffectName.Equals(TEXT("연속 충격")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [연속 충격] 효과로 충격파가 한 번 더 발동됩니다!"));
                UE_LOG(LogTemp, Display, TEXT("    두 번째 충격파 발동! 추가로 %.1f 데미지를 입힙니다."), Skill.Damage * 0.7f);

                // 두 번째 충격파 이펙트 지연 생성
                if (ShockWaveEffect)
                {
                    FTimerHandle SecondEffectTimer;
                    FTimerDelegate TimerDelegate;
                    TimerDelegate.BindLambda([this]()
                                             {
                        FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 150.f;
                        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ShockWaveEffect, SpawnLocation, GetActorRotation()); });

                    GetWorld()->GetTimerManager().SetTimer(SecondEffectTimer, TimerDelegate, 0.5f, false);
                }
            }
            else if (EffectName.Equals(TEXT("충격 침투")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [충격 침투] 효과로 적의 방어력을 20%% 무시합니다!"));
            }
        }

        // 3단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(2) && Skill.TripodTiers[2].bIsUnlocked &&
            Skill.TripodTiers[2].SelectedEffectIndex != -1 &&
            Skill.TripodTiers[2].TripodEffects.IsValidIndex(Skill.TripodTiers[2].SelectedEffectIndex))
        {
            int32 Effect3Index = Skill.TripodTiers[2].SelectedEffectIndex;
            FString EffectName = Skill.TripodTiers[2].TripodEffects[Effect3Index].EffectName;

            if (EffectName.Equals(TEXT("파괴적인 충격")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [파괴적인 충격] 효과로 적중 시 주변에 2차 폭발이 발생합니다!"));
                UE_LOG(LogTemp, Display, TEXT("    2차 폭발 발생! 주변 적들에게 %.1f 데미지를 입힙니다."), Skill.Damage * 0.5f);

                // 2차 폭발 이펙트 생성
                if (ShockWaveEffect)
                {
                    FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 300.f;
                    FVector Scale(1.5f, 1.5f, 1.5f); // 폭발 범위 더 크게

                    FTimerHandle ExplosionTimer;
                    FTimerDelegate TimerDelegate;
                    TimerDelegate.BindLambda([this, SpawnLocation, Scale]()
                                             { UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ShockWaveEffect, SpawnLocation, FRotator::ZeroRotator, Scale); });

                    GetWorld()->GetTimerManager().SetTimer(ExplosionTimer, TimerDelegate, 0.8f, false);
                }
            }
        }
    }
    else if (Skill.SkillName.Equals(TEXT("블레이드 스톰")))
    {
        // 기본 효과
        UE_LOG(LogTemp, Display, TEXT("블레이드 스톰 발동! 주변에 %.1f 데미지의 칼날 폭풍을 일으킵니다."), Skill.Damage);

        // 파티클 이펙트 생성
        if (BladeStormEffect)
        {
            UGameplayStatics::SpawnEmitterAttached(
                BladeStormEffect,
                GetRootComponent(),
                NAME_None,
                FVector::ZeroVector,
                FRotator::ZeroRotator,
                EAttachLocation::SnapToTarget);
        }

        // 트라이포드에 따른 특수 효과
        // 1단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(0) && Skill.TripodTiers[0].bIsUnlocked &&
            Skill.TripodTiers[0].SelectedEffectIndex != -1 &&
            Skill.TripodTiers[0].TripodEffects.IsValidIndex(Skill.TripodTiers[0].SelectedEffectIndex))
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

                // 공격력 버프 적용
                ApplyAttackBuff(10.0f, 1.15f);
            }
        }

        // 2단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(1) && Skill.TripodTiers[1].bIsUnlocked &&
            Skill.TripodTiers[1].SelectedEffectIndex != -1 &&
            Skill.TripodTiers[1].TripodEffects.IsValidIndex(Skill.TripodTiers[1].SelectedEffectIndex))
        {
            int32 Effect2Index = Skill.TripodTiers[1].SelectedEffectIndex;
            FString EffectName = Skill.TripodTiers[1].TripodEffects[Effect2Index].EffectName;

            if (EffectName.Equals(TEXT("화염 블레이드")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [화염 블레이드] 효과로 블레이드가 화염 속성을 가지며 3초간 화상 효과를 줍니다!"));

                // 화상 효과 적용
                ApplyBurnEffect(3.0f, 10.0f);
            }
            else if (EffectName.Equals(TEXT("회복의 칼날")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [회복의 칼날] 효과로 적중한 적 1명당 최대 체력의 1%%를 회복합니다!"));

                // 예시 회복 효과 (실제 적 적중 로직은 게임플레이 상태에 따라 달라질 수 있음)
                UE_LOG(LogTemp, Display, TEXT("    체력을 회복했습니다!"));
            }
        }

        // 3단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(2) && Skill.TripodTiers[2].bIsUnlocked &&
            Skill.TripodTiers[2].SelectedEffectIndex != -1 &&
            Skill.TripodTiers[2].TripodEffects.IsValidIndex(Skill.TripodTiers[2].SelectedEffectIndex))
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

        // 파티클 이펙트 생성
        if (WindBladeEffect)
        {
            FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 50.f + FVector(0, 0, 50.f);
            FRotator SpawnRotation = GetActorRotation();

            UParticleSystemComponent *ParticleComp = UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(),
                WindBladeEffect,
                SpawnLocation,
                SpawnRotation);

            // 파티클이 전방으로 이동하도록 설정
            if (ParticleComp)
            {
                ParticleComp->SetVectorParameter(TEXT("Direction"), GetActorForwardVector());
            }
        }

        // 트라이포드에 따른 특수 효과
        // 1단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(0) && Skill.TripodTiers[0].bIsUnlocked &&
            Skill.TripodTiers[0].SelectedEffectIndex != -1 &&
            Skill.TripodTiers[0].TripodEffects.IsValidIndex(Skill.TripodTiers[0].SelectedEffectIndex))
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
            Skill.TripodTiers[1].SelectedEffectIndex != -1 &&
            Skill.TripodTiers[1].TripodEffects.IsValidIndex(Skill.TripodTiers[1].SelectedEffectIndex))
        {
            int32 Effect2Index = Skill.TripodTiers[1].SelectedEffectIndex;
            FString EffectName = Skill.TripodTiers[1].TripodEffects[Effect2Index].EffectName;

            if (EffectName.Equals(TEXT("바람의 축복")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [바람의 축복] 효과로 5초간 이동 속도가 30%% 증가합니다!"));

                // 이동 속도 버프 적용
                ApplySpeedBuff(5.0f, 1.3f);
            }
            else if (EffectName.Equals(TEXT("삼중 칼날")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [삼중 칼날] 효과로 바람 칼날이 3개로 분열됩니다!"));
                UE_LOG(LogTemp, Display, TEXT("    분열된 칼날 2개가 각각 %.1f 데미지를 추가로 입힙니다."), Skill.Damage * 0.5f);
            }
        }

        // 3단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(2) && Skill.TripodTiers[2].bIsUnlocked &&
            Skill.TripodTiers[2].SelectedEffectIndex != -1 &&
            Skill.TripodTiers[2].TripodEffects.IsValidIndex(Skill.TripodTiers[2].SelectedEffectIndex))
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
    UE_LOG(LogTemp, Warning, TEXT("SelectTripod 호출: 스킬(%d), 티어(%d), 효과(%d)"),
           SkillIndex, TierIndex, EffectIndex);

    if (!Skills.IsValidIndex(SkillIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("SelectTripod 실패: 잘못된 스킬 인덱스 %d"), SkillIndex);
        return;
    }

    FSkillData &Skill = Skills[SkillIndex];

    if (!Skill.TripodTiers.IsValidIndex(TierIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("SelectTripod 실패: 잘못된 티어 인덱스 %d (스킬: %s)"),
               TierIndex, *Skill.SkillName);
        return;
    }

    FTripodTier &Tier = Skill.TripodTiers[TierIndex];

    if (!Tier.TripodEffects.IsValidIndex(EffectIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("SelectTripod 실패: 잘못된 효과 인덱스 %d (스킬: %s, 티어: %d)"),
               EffectIndex, *Skill.SkillName, TierIndex + 1);
        return;
    }

    // 해당 티어가 잠금 해제되었는지 확인
    if (Tier.bIsUnlocked)
    {
        // 이미 선택된 효과와 같은 효과인지 확인
        int32 CurrentSelectedEffect = Tier.SelectedEffectIndex;
        if (CurrentSelectedEffect == EffectIndex)
        {
            // 같은 효과를 다시 선택한 경우, 선택 해제
            Tier.SelectedEffectIndex = -1;
            UE_LOG(LogTemp, Warning, TEXT("트라이포드 선택 해제: 스킬(%s) 티어(%d) 효과(%s)"),
                   *Skill.SkillName,
                   TierIndex + 1,
                   *Tier.TripodEffects[EffectIndex].EffectName);
        }
        else
        {
            // 새로운 효과 선택
            Tier.SelectedEffectIndex = EffectIndex;

            UE_LOG(LogTemp, Warning, TEXT("트라이포드 선택: 스킬(%s) 티어(%d) 효과(%s)"),
                   *Skill.SkillName,
                   TierIndex + 1,
                   *Tier.TripodEffects[EffectIndex].EffectName);
        }

        // 트라이포드 효과 적용
        ApplyTripodEffects(SkillIndex);

        // 현재 선택된 모든 트라이포드 효과 로그 출력
        UE_LOG(LogTemp, Warning, TEXT("=== [%s] 현재 적용 중인 트라이포드 효과 ==="), *Skill.SkillName);
        for (int32 i = 0; i < Skill.TripodTiers.Num(); ++i)
        {
            const FTripodTier &CurrentTier = Skill.TripodTiers[i];
            if (CurrentTier.bIsUnlocked && CurrentTier.SelectedEffectIndex != -1)
            {
                UE_LOG(LogTemp, Warning, TEXT("   티어 %d: %s"),
                       i + 1,
                       *CurrentTier.TripodEffects[CurrentTier.SelectedEffectIndex].EffectName);
            }
            else if (CurrentTier.bIsUnlocked)
            {
                UE_LOG(LogTemp, Warning, TEXT("   티어 %d: 선택된 효과 없음"), i + 1);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("   티어 %d: 잠김"), i + 1);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("트라이포드 티어가 잠겨있습니다. 스킬(%s) 레벨(%d)이 필요 레벨에 도달하지 못했습니다."),
               *Skill.SkillName, Skill.SkillLevel);
    }
}

// 스킬 레벨업 함수
void AMostArkPlayer::LevelUpSkill(int32 SkillIndex)
{
    if (Skills.IsValidIndex(SkillIndex) && SkillPoints > 0)
    {
        FSkillData &Skill = Skills[SkillIndex];
        Skill.SkillLevel++;
        SkillPoints--;

        // 트라이포드 해금 상태 업데이트
        CheckTripodUnlock(SkillIndex);

        UE_LOG(LogTemp, Display, TEXT("스킬 레벨업: %s (레벨 %d)"), *Skill.SkillName, Skill.SkillLevel);
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
    if (!Skills.IsValidIndex(SkillIndex))
        return;

    FSkillData &Skill = Skills[SkillIndex];
    int32 SkillLevel = Skill.SkillLevel;

    // 각 티어별 해금 조건 설정
    for (int32 TierIndex = 0; TierIndex < Skill.TripodTiers.Num(); ++TierIndex)
    {
        FTripodTier &Tier = Skill.TripodTiers[TierIndex];

        // 레벨에 따른 트라이포드 티어 해금
        bool bShouldUnlock = false;

        switch (TierIndex)
        {
        case 0: // 1단계 트라이포드 (레벨 4 이상)
            bShouldUnlock = (SkillLevel >= 4);
            break;
        case 1: // 2단계 트라이포드 (레벨 7 이상)
            bShouldUnlock = (SkillLevel >= 7);
            break;
        case 2: // 3단계 트라이포드 (레벨 10 이상)
            bShouldUnlock = (SkillLevel >= 10);
            break;
        }

        Tier.bIsUnlocked = bShouldUnlock;

        // 디버그 출력
        UE_LOG(LogTemp, Display, TEXT("스킬 [%s] 티어 %d 해금 상태: %s (현재 레벨: %d)"),
               *Skill.SkillName, TierIndex + 1, bShouldUnlock ? TEXT("해금됨") : TEXT("잠김"), SkillLevel);

        // 티어가 해금되었을 경우 선택 가능한 효과 목록 출력
        if (bShouldUnlock)
        {
            UE_LOG(LogTemp, Warning, TEXT("=== [%s] 티어 %d 선택 가능한 효과 ==="),
                   *Skill.SkillName, TierIndex + 1);

            // 각 효과 정보 출력
            for (int32 EffectIndex = 0; EffectIndex < Tier.TripodEffects.Num(); ++EffectIndex)
            {
                if (Tier.TripodEffects.IsValidIndex(EffectIndex))
                {
                    UE_LOG(LogTemp, Warning, TEXT("   %d. %s - %s"),
                           EffectIndex + 1,
                           *Tier.TripodEffects[EffectIndex].EffectName,
                           *Tier.TripodEffects[EffectIndex].Description);
                }
            }
        }
    }
}

// 트라이포드 잠금 확인 함수
void AMostArkPlayer::CheckTripodlock(int32 SkillIndex)
{
    if (Skills.IsValidIndex(SkillIndex))
    {
        FSkillData &Skill = Skills[SkillIndex];

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
        FSkillData &Skill = Skills[SkillIndex];
        Skill.Damage = 100.0f + (Skill.SkillLevel * 10.0f); // 레벨당 기본 데미지 10씩 증가
        Skill.Cooldown = 10.0f;

        // 각 티어의 선택된 효과만 적용 (티어별로 하나씩)
        for (int32 i = 0; i < Skill.TripodTiers.Num(); ++i)
        {
            FTripodTier &Tier = Skill.TripodTiers[i];

            // 해금된 티어이며, 효과가 선택되었는지 확인
            if (Tier.bIsUnlocked && Tier.SelectedEffectIndex != -1 &&
                Tier.TripodEffects.IsValidIndex(Tier.SelectedEffectIndex))
            {
                FTripodEffect &Effect = Tier.TripodEffects[Tier.SelectedEffectIndex];

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

                UE_LOG(LogTemp, Display, TEXT("트라이포드 효과 적용: 티어(%d) - %s"),
                       Tier.TierLevel, *Effect.EffectName);
            }
            else if (Tier.bIsUnlocked && Tier.SelectedEffectIndex == -1)
            {
                UE_LOG(LogTemp, Display, TEXT("트라이포드 티어(%d)에 선택된 효과가 없습니다."),
                       Tier.TierLevel);
            }
        }
    }
}

// 충격파 트라이포드 효과 적용
void AMostArkPlayer::ApplyShockEffects(FSkillData &Skill, int32 TierLevel, const FString &EffectName)
{
    if (TierLevel == 1) // 1단계 트라이포드
    {
        if (EffectName.Equals(TEXT("강화된 충격")))
        {
            Skill.Damage *= 1.3f; // 데미지 30% 증가
        }
        else if (EffectName.Equals(TEXT("넓은 영역")))
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
void AMostArkPlayer::ApplyBladeStormEffects(FSkillData &Skill, int32 TierLevel, const FString &EffectName)
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
            UE_LOG(LogTemp, Display, TEXT("    [무기 강화] 효과로 10초간 공격력이 15%% 증가합니다!"));

            // 공격력 버프 적용
            ApplyAttackBuff(10.0f, 1.15f);
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
void AMostArkPlayer::ApplyWindBladeEffects(FSkillData &Skill, int32 TierLevel, const FString &EffectName)
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
            UE_LOG(LogTemp, Display, TEXT("    [바람의 축복] 효과로 5초간 이동 속도가 30%% 증가합니다!"));

            // 이동 속도 버프 적용
            ApplySpeedBuff(5.0f, 1.3f);
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
            Skill.Damage *= 1.8f;   // 데미지 80% 증가
            Skill.Cooldown *= 1.5f; // 쿨다운 50% 증가
        }
    }
}

// 트라이포드 UI 토글 함수
void AMostArkPlayer::ToggleTripodSystemUI()
{
    if (APlayerController *PC = Cast<APlayerController>(GetController()))
    {
        if (ATripodSystemHUD *TripodHUD = Cast<ATripodSystemHUD>(PC->GetHUD()))
        {
            TripodHUD->ToggleTripodSystemUI();
        }
    }
}

// 트라이포드 효과 설정 함수 추가
void AMostArkPlayer::SetupTripodEffects()
{
    UE_LOG(LogTemp, Display, TEXT("트라이포드 효과 설정 시작"));

    // 각 스킬별로 트라이포드 효과 설정
    for (int32 SkillIndex = 0; SkillIndex < Skills.Num(); ++SkillIndex)
    {
        FSkillData &Skill = Skills[SkillIndex];

        // 각 티어별로 트라이포드 효과 설정
        for (int32 TierIndex = 0; TierIndex < Skill.TripodTiers.Num(); ++TierIndex)
        {
            FTripodTier &Tier = Skill.TripodTiers[TierIndex];

            // 각 티어마다 3개의 효과 추가
            Tier.TripodEffects.SetNum(3);

            // 스킬별로 다른 트라이포드 효과 설정
            if (Skill.SkillName.Equals(TEXT("충격파")))
            {
                if (TierIndex == 0) // 1단계 트라이포드
                {
                    Tier.TripodEffects[0].EffectName = TEXT("강화된 충격");
                    Tier.TripodEffects[0].Description = TEXT("충격파의 데미지가 30% 증가합니다.");

                    Tier.TripodEffects[1].EffectName = TEXT("넓은 영역");
                    Tier.TripodEffects[1].Description = TEXT("충격파의 영향 범위가 40% 증가합니다.");

                    Tier.TripodEffects[2].EffectName = TEXT("빠른 충전");
                    Tier.TripodEffects[2].Description = TEXT("충격파의 쿨다운이 15% 감소합니다.");
                }
                else if (TierIndex == 1) // 2단계 트라이포드
                {
                    Tier.TripodEffects[0].EffectName = TEXT("연속 충격");
                    Tier.TripodEffects[0].Description = TEXT("충격파가 한 번 더 발사됩니다.");

                    Tier.TripodEffects[1].EffectName = TEXT("충격 침투");
                    Tier.TripodEffects[1].Description = TEXT("충격파가 적의 방어력을 20% 무시합니다.");

                    Tier.TripodEffects[2].EffectName = TEXT("빠른 시전");
                    Tier.TripodEffects[2].Description = TEXT("시전 속도가 30% 증가합니다.");
                }
                else if (TierIndex == 2) // 3단계 트라이포드
                {
                    Tier.TripodEffects[0].EffectName = TEXT("파괴적인 충격");
                    Tier.TripodEffects[0].Description = TEXT("충격파가 적중하면 주변에 2차 폭발을 일으킵니다.");

                    Tier.TripodEffects[1].EffectName = TEXT("전기 충격");
                    Tier.TripodEffects[1].Description = TEXT("충격파에 전기 속성이 추가되어 5초간 도트 데미지를 입힙니다.");

                    Tier.TripodEffects[2].EffectName = TEXT("멀티 충격");
                    Tier.TripodEffects[2].Description = TEXT("충격파가 여러 개로 나뉘어 발사됩니다.");
                }
            }
            else if (Skill.SkillName.Equals(TEXT("블레이드 스톰")))
            {
                if (TierIndex == 0) // 1단계 트라이포드
                {
                    Tier.TripodEffects[0].EffectName = TEXT("강화된 블레이드");
                    Tier.TripodEffects[0].Description = TEXT("블레이드 스톰의 데미지가 25% 증가합니다.");

                    Tier.TripodEffects[1].EffectName = TEXT("지속 시간 증가");
                    Tier.TripodEffects[1].Description = TEXT("블레이드 스톰의 지속 시간이 30% 증가합니다.");

                    Tier.TripodEffects[2].EffectName = TEXT("무기 강화");
                    Tier.TripodEffects[2].Description = TEXT("사용 후 10초간 공격력이 15% 증가합니다.");
                }
                else if (TierIndex == 1) // 2단계 트라이포드
                {
                    Tier.TripodEffects[0].EffectName = TEXT("화염 블레이드");
                    Tier.TripodEffects[0].Description = TEXT("블레이드가 화염 속성을 가지며 화상 효과를 줍니다.");

                    Tier.TripodEffects[1].EffectName = TEXT("회복의 칼날");
                    Tier.TripodEffects[1].Description = TEXT("적중한 적 1명당 최대 체력의 1%를 회복합니다.");

                    Tier.TripodEffects[2].EffectName = TEXT("소용돌이 증가");
                    Tier.TripodEffects[2].Description = TEXT("블레이드 스톰의 회전 속도가 증가하여 타격 횟수가 늘어납니다.");
                }
                else if (TierIndex == 2) // 3단계 트라이포드
                {
                    Tier.TripodEffects[0].EffectName = TEXT("블레이드 춤");
                    Tier.TripodEffects[0].Description = TEXT("블레이드 스톰 종료 후 전방으로 도약하며 강력한 일격을 가합니다.");

                    Tier.TripodEffects[1].EffectName = TEXT("죽음의 소용돌이");
                    Tier.TripodEffects[1].Description = TEXT("블레이드 스톰의 크기가 50% 증가하고 중심부에서 더 강한 데미지를 입힙니다.");

                    Tier.TripodEffects[2].EffectName = TEXT("블레이드 폭풍");
                    Tier.TripodEffects[2].Description = TEXT("블레이드 스톰에서 칼날이 튀어나가 주변 적을 추가로 공격합니다.");
                }
            }
            else if (Skill.SkillName.Equals(TEXT("윈드 블레이드")))
            {
                if (TierIndex == 0) // 1단계 트라이포드
                {
                    Tier.TripodEffects[0].EffectName = TEXT("관통 바람");
                    Tier.TripodEffects[0].Description = TEXT("윈드 블레이드가 적을 관통합니다.");

                    Tier.TripodEffects[1].EffectName = TEXT("빠른 발사");
                    Tier.TripodEffects[1].Description = TEXT("시전 속도가 30% 증가합니다.");

                    Tier.TripodEffects[2].EffectName = TEXT("예리한 칼날");
                    Tier.TripodEffects[2].Description = TEXT("크리티컬 확률이 15% 증가합니다.");
                }
                else if (TierIndex == 1) // 2단계 트라이포드
                {
                    Tier.TripodEffects[0].EffectName = TEXT("빙결 칼날");
                    Tier.TripodEffects[0].Description = TEXT("윈드 블레이드가 얼음 속성을 가지며 적의 이동 속도를 감소시킵니다.");

                    Tier.TripodEffects[1].EffectName = TEXT("연속 발사");
                    Tier.TripodEffects[1].Description = TEXT("윈드 블레이드를 연속해서 2번 발사합니다.");

                    Tier.TripodEffects[2].EffectName = TEXT("정신 집중");
                    Tier.TripodEffects[2].Description = TEXT("치명타 데미지가 30% 증가합니다.");
                }
                else if (TierIndex == 2) // 3단계 트라이포드
                {
                    Tier.TripodEffects[0].EffectName = TEXT("회오리 칼날");
                    Tier.TripodEffects[0].Description = TEXT("윈드 블레이드가 회오리 형태로 발사되어 주변 적들을 끌어당깁니다.");

                    Tier.TripodEffects[1].EffectName = TEXT("제압 바람");
                    Tier.TripodEffects[1].Description = TEXT("적중된 적이 3초간 기절 상태가 됩니다.");

                    Tier.TripodEffects[2].EffectName = TEXT("선풍 칼날");
                    Tier.TripodEffects[2].Description = TEXT("윈드 블레이드 발사 후 바람 기운이 폭발하여 주변에 추가 데미지를 입힙니다.");
                }
            }
        }
    }

    UE_LOG(LogTemp, Display, TEXT("트라이포드 효과 설정 완료"));
}

// 개별 스킬 사용 함수 구현
void AMostArkPlayer::UseSkill1()
{
    if (!IsSkillOnCooldown(0))
    {
        UseSkill(0);
        ResetSkillCooldown(0);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("스킬 1(%s)이 쿨다운 중입니다!"), *Skills[0].SkillName);
    }
}

void AMostArkPlayer::UseSkill2()
{
    if (!IsSkillOnCooldown(1))
    {
        UseSkill(1);
        ResetSkillCooldown(1);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("스킬 2(%s)이 쿨다운 중입니다!"), *Skills[1].SkillName);
    }
}

void AMostArkPlayer::UseSkill3()
{
    if (!IsSkillOnCooldown(2))
    {
        UseSkill(2);
        ResetSkillCooldown(2);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("스킬 3(%s)이 쿨다운 중입니다!"), *Skills[2].SkillName);
    }
}

// 스킬 쿨다운 체크 함수
bool AMostArkPlayer::IsSkillOnCooldown(int32 SkillIndex) const
{
    if (Skills.IsValidIndex(SkillIndex))
    {
        return !bSkillAvailable[SkillIndex];
    }
    return true; // 유효하지 않은 인덱스는 항상 쿨다운 중인 것으로 처리
}

// 쿨다운 완료 콜백 함수
void AMostArkPlayer::OnSkillCooldownComplete(int32 SkillIndex)
{
    if (Skills.IsValidIndex(SkillIndex))
    {
        bSkillAvailable[SkillIndex] = true;
        UE_LOG(LogTemp, Display, TEXT("스킬 %s의 쿨다운이 완료되었습니다!"), *Skills[SkillIndex].SkillName);
    }
}

// 스킬 쿨다운 리셋 함수
void AMostArkPlayer::ResetSkillCooldown(int32 SkillIndex)
{
    if (Skills.IsValidIndex(SkillIndex))
    {
        bSkillAvailable[SkillIndex] = false;

        // 실제 쿨다운 시간 설정
        float CooldownTime = Skills[SkillIndex].Cooldown;

        // 타이머 설정
        GetWorld()->GetTimerManager().SetTimer(
            SkillCooldownTimers[SkillIndex],
            FTimerDelegate::CreateUObject(this, &AMostArkPlayer::OnSkillCooldownComplete, SkillIndex),
            CooldownTime,
            false);

        UE_LOG(LogTemp, Display, TEXT("스킬 %s의 쿨다운 시작 (%.1f초)"), *Skills[SkillIndex].SkillName, CooldownTime);
    }
}

// 스피드 버프 적용 함수
void AMostArkPlayer::ApplySpeedBuff(float Duration, float Multiplier)
{
    if (bHasSpeedBuff)
    {
        // 이미 버프가 적용 중이면 타이머만 갱신
        GetWorld()->GetTimerManager().ClearTimer(SpeedBuffTimer);
    }
    else
    {
        // 새로운 버프 적용
        bHasSpeedBuff = true;
        OriginalMovementSpeed = GetCharacterMovement()->MaxWalkSpeed;
        GetCharacterMovement()->MaxWalkSpeed = OriginalMovementSpeed * Multiplier;

        UE_LOG(LogTemp, Display, TEXT("이동 속도 버프 적용! (%.1f초 동안 %.1f배)"), Duration, Multiplier);
    }

    // 버프 종료 타이머 설정
    GetWorld()->GetTimerManager().SetTimer(
        SpeedBuffTimer,
        this,
        &AMostArkPlayer::RemoveSpeedBuff,
        Duration,
        false);
}

// 스피드 버프 제거 함수
void AMostArkPlayer::RemoveSpeedBuff()
{
    if (bHasSpeedBuff)
    {
        bHasSpeedBuff = false;
        GetCharacterMovement()->MaxWalkSpeed = OriginalMovementSpeed;

        UE_LOG(LogTemp, Display, TEXT("이동 속도 버프 종료!"));
    }
}

// 화상 효과 적용 함수
void AMostArkPlayer::ApplyBurnEffect(float Duration, float DamagePerTick)
{
    // 이미 화상 효과가 적용 중이면 타이머 갱신
    if (GetWorld()->GetTimerManager().IsTimerActive(BurnEffectTimer))
    {
        GetWorld()->GetTimerManager().ClearTimer(BurnEffectTimer);
        BurnEffectCount = 0;
    }

    // 0.5초마다 데미지를 입히는 화상 효과
    BurnEffectCount = FMath::RoundToInt(Duration / 0.5f);

    UE_LOG(LogTemp, Display, TEXT("화상 효과 적용! (%.1f초 동안 매 0.5초마다 %.1f 데미지)"), Duration, DamagePerTick);

    // 화상 효과 타이머 설정
    GetWorld()->GetTimerManager().SetTimer(
        BurnEffectTimer,
        FTimerDelegate::CreateUObject(this, &AMostArkPlayer::ApplyBurnEffectTick, DamagePerTick),
        0.5f,
        true);
}

// 화상 효과 틱 함수
void AMostArkPlayer::ApplyBurnEffectTick(float DamagePerTick)
{
    if (BurnEffectCount > 0)
    {
        // 플레이어에게 화상 데미지 적용
        UE_LOG(LogTemp, Display, TEXT("화상 효과로 %.1f 데미지를 입었습니다!"), DamagePerTick);

        // 여기에 실제 데미지 적용 코드 추가 가능
        // TakeDamage(DamagePerTick, FDamageEvent(), GetController(), this);

        BurnEffectCount--;

        // 화상 효과 종료 확인
        if (BurnEffectCount <= 0)
        {
            RemoveBurnEffect();
        }
    }
}

// 화상 효과 제거 함수
void AMostArkPlayer::RemoveBurnEffect()
{
    GetWorld()->GetTimerManager().ClearTimer(BurnEffectTimer);
    BurnEffectCount = 0;

    UE_LOG(LogTemp, Display, TEXT("화상 효과 종료!"));
}

// 공격력 버프 적용 함수
void AMostArkPlayer::ApplyAttackBuff(float Duration, float Multiplier)
{
    if (bHasAttackBuff)
    {
        // 이미 버프가 적용 중이면 타이머만 갱신
        GetWorld()->GetTimerManager().ClearTimer(AttackBuffTimer);
    }
    else
    {
        // 새로운 버프 적용
        bHasAttackBuff = true;
        OriginalDamageMultiplier = 1.0f;

        UE_LOG(LogTemp, Display, TEXT("공격력 버프 적용! (%.1f초 동안 %.1f배)"), Duration, Multiplier);

        // 실제 적용 로직은 스킬 사용 시 ApplyTripodEffects에서 처리
    }

    // 버프 종료 타이머 설정
    GetWorld()->GetTimerManager().SetTimer(
        AttackBuffTimer,
        this,
        &AMostArkPlayer::RemoveAttackBuff,
        Duration,
        false);
}

// 공격력 버프 제거 함수
void AMostArkPlayer::RemoveAttackBuff()
{
    if (bHasAttackBuff)
    {
        bHasAttackBuff = false;

        UE_LOG(LogTemp, Display, TEXT("공격력 버프 종료!"));
    }
}