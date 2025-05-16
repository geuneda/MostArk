// Fill out your copyright notice in the Description page of Project Settings.

#include "MostArkPlayer.h"

#include "NiagaraFunctionLibrary.h"
#include "Components/InputComponent.h"
#include "../HUD/TripodSystemHUD.h"
#include "ArkProject/Widget/GameOverWidget.h"
#include "ArkProject/Widget/PlayerHPWidget.h"
#include "ArkProject/Widget/PlayerSkillWidget.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"

AMostArkPlayer::AMostArkPlayer()
{
    PrimaryActorTick.bCanEverTick = true;

    WeaponComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponComp"));
    WeaponComp->SetupAttachment(GetMesh(), TEXT("Sword"));

    ConstructorHelpers::FObjectFinder<UStaticMesh> tempWeaponMesh(TEXT("/Script/Engine.StaticMesh'/Game/Assassin/Mesh/Separated_Mesh/Weapon/SM_sword_Sword.SM_sword_Sword'"));
    if (tempWeaponMesh.Succeeded())
    {
        WeaponComp->SetStaticMesh(tempWeaponMesh.Object);
    }

    GetCharacterMovement()->MaxAcceleration = 10000.f;
    GetCharacterMovement()->BrakingFrictionFactor = 0.f;
    FSkillData ShockSkill;
    ShockSkill.SkillName = TEXT("베기");
    ShockSkill.SkillLevel = 1;
    Skills.Add(ShockSkill);

    FSkillData BladeStormSkill;
    BladeStormSkill.SkillName = TEXT("발차기");
    BladeStormSkill.SkillLevel = 1;
    Skills.Add(BladeStormSkill);

    FSkillData WindBladeSkill;
    WindBladeSkill.SkillName = TEXT("회전베기");
    WindBladeSkill.SkillLevel = 1;
    Skills.Add(WindBladeSkill);

    // 기본 선택 스킬 설정
    SelectedSkillIndex = 0;

    // 초기 스킬 포인트 설정
    SkillPoints = 30;

    // 트라이포드 효과 초기화
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

    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
    GetCharacterMovement()->bConstrainToPlane = true;
    GetCharacterMovement()->bSnapToPlaneAtStart = true;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->SetUsingAbsoluteRotation(true);
    CameraBoom->TargetArmLength = 800.f;
    CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
    CameraBoom->bDoCollisionTest = false;

    TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
    TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    TopDownCameraComponent->bUsePawnControlRotation = false;

    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    // 공격용 콜리전 컴포넌트 생성 및 초기화 
    SwordCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("SwordCollision"));
    SwordCollision->SetupAttachment(GetMesh(), TEXT("Sword"));
    SwordCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SwordCollision->SetCollisionObjectType(ECC_WorldDynamic);
    SwordCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    SwordCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    KickCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("KickCollision"));
    KickCollision->SetupAttachment(GetMesh(), TEXT("foot_r"));
    KickCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    KickCollision->SetCollisionObjectType(ECC_WorldDynamic);
    KickCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    KickCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // 밀림 현상 방지
    GetCharacterMovement()->bEnablePhysicsInteraction = false;
    GetCharacterMovement()->PushForceFactor = 0;
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
    SkillCooldownRemaining.Init(0.0f, Skills.Num());

    // 특수 효과 변수 초기화
    bHasSpeedBuff = false;
    OriginalMovementSpeed = GetCharacterMovement()->MaxWalkSpeed;
    bHasAttackBuff = false;
    OriginalDamageMultiplier = 1.0f;
    BurnEffectCount = 0;

    // 콜리전 오버랩 이벤트 바인딩
    if (SwordCollision)
    {
        SwordCollision->OnComponentBeginOverlap.AddDynamic(this, &AMostArkPlayer::OnSwordCollisionBeginOverlap);
    }
    if (KickCollision)
    {
        KickCollision->OnComponentBeginOverlap.AddDynamic(this, &AMostArkPlayer::OnKickCollisionBeginOverlap);
    }

    UE_LOG(LogTemp, Warning, TEXT("MostArkPlayer BeginPlay 완료"));

    // 스킬 위젯 생성 및 초기화
    if (SkillWidgetFactory)
    {
        SkillWidget = CreateWidget<UPlayerSkillWidget>(GetWorld(), SkillWidgetFactory);
        if (SkillWidget)
        {
            SkillWidget->AddToViewport(0);
            
            // 쿨다운 텍스트 초기화
            for (int32 i = 0; i < Skills.Num(); ++i)
            {
                SkillWidget->ResetSkillCooldownText(i);
            }
        }
    }

    if (HPWidgetFactory)
    {
        HPWidget = CreateWidget<UPlayerHPWidget>(GetWorld(), HPWidgetFactory);
        if (HPWidget)
        {
            HPWidget->AddToViewport(0);
            HPWidget->SetHPPercent(CurrentHP, MaxHP);
        }
    }
    
    // 쿨다운 텍스트 업데이트 타이머 설정 (0.1초마다 업데이트)
    GetWorld()->GetTimerManager().SetTimer(
        CooldownUpdateTimerHandle,
        this,
        &AMostArkPlayer::UpdateCooldownTexts,
        0.1f,
        true);

    CurrentHP = MaxHP;
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

void AMostArkPlayer::GameOver()
{
    GameOverWidget = CreateWidget<UGameOverWidget>(GetWorld(), GameOverWidgetFactory);
    GameOverWidget->AddToViewport(0);
}

float AMostArkPlayer::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                                 class AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHP = FMath::Max(0.0f, CurrentHP - ActualDamage);

    HPWidget->SetHPPercent(CurrentHP, MaxHP);

    if (CurrentHP <= 0.0f)
    {
        GameOver();
    }

    return ActualDamage;
}

// 스킬 사용 함수
void AMostArkPlayer::UseSkill(int32 SkillIndex)
{
    if (Skills.IsValidIndex(SkillIndex))
    {
        // 스킬 사용 전 트라이포드 효과 적용
        ApplyTripodEffects(SkillIndex);

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
    if (SkillIndex < 0 || SkillIndex >= Skills.Num())
        return;

    // 현재 활성화된 스킬 인덱스 설정
    ActiveSkillIndex = SkillIndex;

    FSkillData& Skill = Skills[SkillIndex];
    UE_LOG(LogTemp, Warning, TEXT("스킬 실행: %s"), *Skill.SkillName);

    // 스킬별 효과 적용
    if (Skill.SkillName == TEXT("베기"))
    {
        ActivateSwordCollision(true);
        
        // 타이머로 콜리전 비활성화 (0.5초 후)
        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]()
        {
            ActivateSwordCollision(false);
            ActiveSkillIndex = -1; // 스킬 종료
        }), 0.5f, false);

        UE_LOG(LogTemp, Display, TEXT("베기 발동! 전방에 %.1f 데미지의 베기를 발사합니다."), Skill.Damage);

        if (Skill1AnimMontage)
        {
            auto* animIns = GetMesh()->GetAnimInstance();
            animIns->Montage_Play(Skill1AnimMontage, Skill1AnimMontageSpeed);
        }

        // 트라이포드에 따른 특수 효과 - 티어별로 선택된 효과만 적용
        // 1단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(0) && Skill.TripodTiers[0].bIsUnlocked &&
            Skill.TripodTiers[0].SelectedEffectIndex != -1 &&
            Skill.TripodTiers[0].TripodEffects.IsValidIndex(Skill.TripodTiers[0].SelectedEffectIndex))
        {
            int32 Effect1Index = Skill.TripodTiers[0].SelectedEffectIndex;
            FString EffectName = Skill.TripodTiers[0].TripodEffects[Effect1Index].EffectName;

            if (EffectName.Equals(TEXT("강화된 베기")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [강화된 베기] 효과로 데미지가 30%% 증가했습니다!"));
            }
            else if (EffectName.Equals(TEXT("넓은 베기")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [넓은 베기] 효과로 공격 범위가 40%% 증가했습니다!"));
            }
            else if (EffectName.Equals(TEXT("빠른 베기")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [빠른 베기] 효과로 쿨다운이 15%% 감소했습니다!"));
            }
        }

        // 2단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(1) && Skill.TripodTiers[1].bIsUnlocked &&
            Skill.TripodTiers[1].SelectedEffectIndex != -1 &&
            Skill.TripodTiers[1].TripodEffects.IsValidIndex(Skill.TripodTiers[1].SelectedEffectIndex))
        {
            int32 Effect2Index = Skill.TripodTiers[1].SelectedEffectIndex;
            FString EffectName = Skill.TripodTiers[1].TripodEffects[Effect2Index].EffectName;

            if (EffectName.Equals(TEXT("이중 베기")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [이중 베기] 효과로 베기가 한 번 더 발동됩니다!"));
                UE_LOG(LogTemp, Display, TEXT("    두 번째 베기 발동! 추가로 %.1f 데미지를 입힙니다."), Skill.Damage * 0.7f);
            }
            else if (EffectName.Equals(TEXT("관통 베기")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [관통 베기] 효과로 적의 방어력을 20%% 무시합니다!"));
            }
        }

        // 3단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(2) && Skill.TripodTiers[2].bIsUnlocked &&
            Skill.TripodTiers[2].SelectedEffectIndex != -1 &&
            Skill.TripodTiers[2].TripodEffects.IsValidIndex(Skill.TripodTiers[2].SelectedEffectIndex))
        {
            int32 Effect3Index = Skill.TripodTiers[2].SelectedEffectIndex;
            FString EffectName = Skill.TripodTiers[2].TripodEffects[Effect3Index].EffectName;

            if (EffectName.Equals(TEXT("파괴적 베기")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [파괴적 베기] 효과로 적중 시 주변에 2차 폭발이 발생합니다!"));
                UE_LOG(LogTemp, Display, TEXT("    2차 폭발 발생! 주변 적들에게 %.1f 데미지를 입힙니다."), Skill.Damage * 0.5f);
            }
        }
    }
    else if (Skill.SkillName == TEXT("발차기"))
    {
        ActivateKickCollision(true);
        
        // 타이머로 콜리전 비활성화 (0.3초 후)
        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]()
        {
            ActivateKickCollision(false);
            ActiveSkillIndex = -1; // 스킬 종료
        }), 0.3f, false);

        UE_LOG(LogTemp, Display, TEXT("발차기 발동! 주변에 %.1f 데미지의 발차기를 일으킵니다."), Skill.Damage);

        if (Skill2AnimMontage)
        {
            auto* animIns = GetMesh()->GetAnimInstance();
            animIns->Montage_Play(Skill2AnimMontage, Skill2AnimMontageSpeed);
        }
        
        // 트라이포드에 따른 특수 효과
        // 1단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(0) && Skill.TripodTiers[0].bIsUnlocked &&
            Skill.TripodTiers[0].SelectedEffectIndex != -1 &&
            Skill.TripodTiers[0].TripodEffects.IsValidIndex(Skill.TripodTiers[0].SelectedEffectIndex))
        {
            int32 Effect1Index = Skill.TripodTiers[0].SelectedEffectIndex;
            FString EffectName = Skill.TripodTiers[0].TripodEffects[Effect1Index].EffectName;

            if (EffectName.Equals(TEXT("강화된 발차기")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [강화된 발차기] 효과로 데미지가 25%% 증가했습니다!"));
            }
            else if (EffectName.Equals(TEXT("넓은 발차기")))
            {
                // 범위 증가는 ExecuteSkillEffect에서 구현
            }
            else if (EffectName.Equals(TEXT("빠른 발차기")))
            {
                Skill.Cooldown *= 0.85f; // 쿨다운 15% 감소
            }
        }

        // 2단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(1) && Skill.TripodTiers[1].bIsUnlocked &&
            Skill.TripodTiers[1].SelectedEffectIndex != -1 &&
            Skill.TripodTiers[1].TripodEffects.IsValidIndex(Skill.TripodTiers[1].SelectedEffectIndex))
        {
            int32 Effect2Index = Skill.TripodTiers[1].SelectedEffectIndex;
            FString EffectName = Skill.TripodTiers[1].TripodEffects[Effect2Index].EffectName;

            if (EffectName.Equals(TEXT("이중 발차기")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [이중 발차기] 효과로 발차기가 한 번 더 발동됩니다!"));
                UE_LOG(LogTemp, Display, TEXT("    두 번째 발차기 발동! 추가로 %.1f 데미지를 입힙니다."), Skill.Damage * 0.7f);
            }
            else if (EffectName.Equals(TEXT("관통 발차기")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [관통 발차기] 효과로 적의 방어력을 20%% 무시합니다!"));
            }
        }

        // 3단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(2) && Skill.TripodTiers[2].bIsUnlocked &&
            Skill.TripodTiers[2].SelectedEffectIndex != -1 &&
            Skill.TripodTiers[2].TripodEffects.IsValidIndex(Skill.TripodTiers[2].SelectedEffectIndex))
        {
            int32 Effect3Index = Skill.TripodTiers[2].SelectedEffectIndex;
            FString EffectName = Skill.TripodTiers[2].TripodEffects[Effect3Index].EffectName;

            if (EffectName.Equals(TEXT("파괴적 발차기")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [파괴적 발차기] 효과로 적중 시 주변에 2차 폭발을 일으킵니다!"));
                UE_LOG(LogTemp, Display, TEXT("    2차 폭발 발생! 주변 적들에게 %.1f 데미지를 입힙니다."), Skill.Damage * 0.5f);
            }
        }
    }
    else if (Skill.SkillName == TEXT("회전베기"))
    {
        ActivateSwordCollision(true);
        
        // 타이머로 콜리전 비활성화 (0.7초 후)
        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]()
        {
            ActivateSwordCollision(false);
            ActiveSkillIndex = -1; // 스킬 종료
        }), 0.7f, false);

        UE_LOG(LogTemp, Display, TEXT("회전베기 발동! 주변에 %.1f 데미지의 회전베기를 일으킵니다."), Skill.Damage);

        if (Skill3AnimMontage)
        {
            auto* animIns = GetMesh()->GetAnimInstance();
            animIns->Montage_Play(Skill3AnimMontage, Skill3AnimMontageSpeed);
        }

        // 트라이포드에 따른 특수 효과
        // 1단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(0) && Skill.TripodTiers[0].bIsUnlocked &&
            Skill.TripodTiers[0].SelectedEffectIndex != -1 &&
            Skill.TripodTiers[0].TripodEffects.IsValidIndex(Skill.TripodTiers[0].SelectedEffectIndex))
        {
            int32 Effect1Index = Skill.TripodTiers[0].SelectedEffectIndex;
            FString EffectName = Skill.TripodTiers[0].TripodEffects[Effect1Index].EffectName;

            if (EffectName.Equals(TEXT("강화된 회전베기")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [강화된 회전베기] 효과로 데미지가 25%% 증가했습니다!"));
            }
            else if (EffectName.Equals(TEXT("넓은 회전베기")))
            {
                // 범위 증가는 ExecuteSkillEffect에서 구현
            }
            else if (EffectName.Equals(TEXT("빠른 회전베기")))
            {
                Skill.Cooldown *= 0.85f; // 쿨다운 15% 감소
            }
        }

        // 2단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(1) && Skill.TripodTiers[1].bIsUnlocked &&
            Skill.TripodTiers[1].SelectedEffectIndex != -1 &&
            Skill.TripodTiers[1].TripodEffects.IsValidIndex(Skill.TripodTiers[1].SelectedEffectIndex))
        {
            int32 Effect2Index = Skill.TripodTiers[1].SelectedEffectIndex;
            FString EffectName = Skill.TripodTiers[1].TripodEffects[Effect2Index].EffectName;

            if (EffectName.Equals(TEXT("이중 회전베기")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [이중 회전베기] 효과로 회전베기가 한 번 더 발동됩니다!"));
                UE_LOG(LogTemp, Display, TEXT("    두 번째 회전베기 발동! 추가로 %.1f 데미지를 입힙니다."), Skill.Damage * 0.7f);
            }
            else if (EffectName.Equals(TEXT("관통 회전베기")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [관통 회전베기] 효과로 적의 방어력을 20%% 무시합니다!"));
            }
        }

        // 3단계 트라이포드 효과 확인
        if (Skill.TripodTiers.IsValidIndex(2) && Skill.TripodTiers[2].bIsUnlocked &&
            Skill.TripodTiers[2].SelectedEffectIndex != -1 &&
            Skill.TripodTiers[2].TripodEffects.IsValidIndex(Skill.TripodTiers[2].SelectedEffectIndex))
        {
            int32 Effect3Index = Skill.TripodTiers[2].SelectedEffectIndex;
            FString EffectName = Skill.TripodTiers[2].TripodEffects[Effect3Index].EffectName;

            if (EffectName.Equals(TEXT("파괴적 회전베기")))
            {
                UE_LOG(LogTemp, Display, TEXT("    [파괴적 회전베기] 효과로 적중 시 주변에 2차 폭발을 일으킵니다!"));
                UE_LOG(LogTemp, Display, TEXT("    2차 폭발 발생! 주변 적들에게 %.1f 데미지를 입힙니다."), Skill.Damage * 0.5f);
            }
        }
    }
}

// 스킬 데미지 계산 함수 구현
float AMostArkPlayer::CalculateSkillDamage(int32 SkillIndex)
{
    if (SkillIndex < 0 || SkillIndex >= Skills.Num())
    {
        return 10.0f; // 기본값
    }

    FSkillData& Skill = Skills[SkillIndex];
    
    // 기본 데미지 설정 (스킬별로 다른 기본값)
    float BaseDamage = 0.0f;
    if (Skill.SkillName == TEXT("베기"))
    {
        BaseDamage = 50.0f;
    }
    else if (Skill.SkillName == TEXT("발차기"))
    {
        BaseDamage = 30.0f;
    }
    else if (Skill.SkillName == TEXT("회전베기"))
    {
        BaseDamage = 70.0f;
    }
    else
    {
        BaseDamage = 20.0f;
    }
    
    // 스킬 레벨에 따른 데미지 증가 (레벨당 10% 증가)
    BaseDamage *= (1.0f + (Skill.SkillLevel - 1) * 0.1f);
    
    // 트라이포드 효과 적용
    for (int32 TierIndex = 0; TierIndex < Skill.TripodTiers.Num(); ++TierIndex)
    {
        const FTripodTier& Tier = Skill.TripodTiers[TierIndex];
        if (Tier.bIsUnlocked && Tier.SelectedEffectIndex >= 0 && Tier.SelectedEffectIndex < Tier.TripodEffects.Num())
        {
            const FTripodEffect& Effect = Tier.TripodEffects[Tier.SelectedEffectIndex];
            
            // 데미지 증가 효과 확인 (효과 이름에 "데미지" 또는 "공격력" 포함된 경우)
            if (Effect.EffectName.Contains(TEXT("데미지")) || Effect.EffectName.Contains(TEXT("공격력")))
            {
                // 티어 레벨에 따라 데미지 증가 (티어1: 20%, 티어2: 40%, 티어3: 60%)
                DamageMultiplier = 1.0f + (Tier.TierLevel * 0.2f);
                BaseDamage *= DamageMultiplier;
            }
        }
    }
    
    // 공격력 버프 적용
    BaseDamage *= DamageMultiplier;
    
    return BaseDamage;
}

// 콜리전 활성화/비활성화 함수 구현
void AMostArkPlayer::ActivateSwordCollision(bool bActivate)
{
    if (SwordCollision)
    {
        SwordCollision->SetCollisionEnabled(bActivate ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
        
        // 콜리전이 활성화될 때 데미지 추적 플래그 초기화
        if (bActivate)
        {
            bDamageAppliedForSwordAnimation = false;
        }
    }
}

void AMostArkPlayer::ActivateKickCollision(bool bActivate)
{
    if (KickCollision)
    {
        KickCollision->SetCollisionEnabled(bActivate ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
        
        // 콜리전이 활성화될 때 데미지 추적 플래그 초기화
        if (bActivate)
        {
            bDamageAppliedForKickAnimation = false;
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
        default:
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
                if (Skill.SkillName.Equals(TEXT("베기")))
                {
                    ApplyShockEffects(Skill, Tier.TierLevel, Effect.EffectName);
                }
                else if (Skill.SkillName.Equals(TEXT("발차기")))
                {
                    ApplyBladeStormEffects(Skill, Tier.TierLevel, Effect.EffectName);
                }
                else if (Skill.SkillName.Equals(TEXT("회전베기")))
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
        if (EffectName.Equals(TEXT("강화된 베기")))
        {
            Skill.Damage *= 1.3f; // 데미지 30% 증가
        }
        else if (EffectName.Equals(TEXT("넓은 베기")))
        {
            // 범위 증가 효과는 ExecuteSkillEffect에서 구현
        }
        else if (EffectName.Equals(TEXT("빠른 베기")))
        {
            Skill.Cooldown *= 0.85f; // 쿨다운 15% 감소
        }
    }
    else if (TierLevel == 2) // 2단계 트라이포드
    {
        if (EffectName.Equals(TEXT("이중 베기")))
        {
            Skill.Damage *= 1.7f; // 1회차 100% + 2회차 70%
        }
        else if (EffectName.Equals(TEXT("관통 베기")))
        {
            // 방어력 무시는 ExecuteSkillEffect에서 구현
        }
    }
    else if (TierLevel == 3) // 3단계 트라이포드
    {
        if (EffectName.Equals(TEXT("파괴적 베기")))
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
        if (EffectName.Equals(TEXT("강화된 발차기")))
        {
            Skill.Damage *= 1.25f; // 데미지 25% 증가
        }
        else if (EffectName.Equals(TEXT("넓은 발차기")))
        {
            // 범위 증가는 ExecuteSkillEffect에서 구현
        }
        else if (EffectName.Equals(TEXT("빠른 발차기")))
        {
            Skill.Cooldown *= 0.85f; // 쿨다운 15% 감소
        }
    }
    else if (TierLevel == 2) // 2단계 트라이포드
    {
        if (EffectName.Equals(TEXT("이중 발차기")))
        {
            Skill.Damage *= 1.7f; // 1회차 100% + 2회차 70%
        }
        else if (EffectName.Equals(TEXT("관통 발차기")))
        {
            // 방어력 무시는 ExecuteSkillEffect에서 구현
        }
    }
    else if (TierLevel == 3) // 3단계 트라이포드
    {
        if (EffectName.Equals(TEXT("파괴적 발차기")))
        {
            Skill.Damage *= 1.5f; // 2차 폭발을 포함한 총 데미지
        }
    }
}

// 윈드 블레이드 트라이포드 효과 적용
void AMostArkPlayer::ApplyWindBladeEffects(FSkillData &Skill, int32 TierLevel, const FString &EffectName)
{
    if (TierLevel == 1) // 1단계 트라이포드
    {
        if (EffectName.Equals(TEXT("강화된 회전베기")))
        {
            Skill.Damage *= 1.25f; // 데미지 25% 증가
        }
        else if (EffectName.Equals(TEXT("넓은 회전베기")))
        {
            // 범위 증가는 ExecuteSkillEffect에서 구현
        }
        else if (EffectName.Equals(TEXT("빠른 회전베기")))
        {
            Skill.Cooldown *= 0.85f; // 쿨다운 15% 감소
        }
    }
    else if (TierLevel == 2) // 2단계 트라이포드
    {
        if (EffectName.Equals(TEXT("이중 회전베기")))
        {
            Skill.Damage *= 1.7f; // 1회차 100% + 2회차 70%
        }
        else if (EffectName.Equals(TEXT("관통 회전베기")))
        {
            // 방어력 무시는 ExecuteSkillEffect에서 구현
        }
    }
    else if (TierLevel == 3) // 3단계 트라이포드
    {
        if (EffectName.Equals(TEXT("파괴적 회전베기")))
        {
            Skill.Damage *= 1.5f; // 2차 폭발을 포함한 총 데미지
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
            if (Skill.SkillName.Equals(TEXT("베기")))
            {
                if (TierIndex == 0) // 1단계 트라이포드
                {
                    Tier.TripodEffects[0].EffectName = TEXT("강화된 베기");
                    Tier.TripodEffects[0].Description = TEXT("베기의 데미지가 30% 증가합니다.");

                    Tier.TripodEffects[1].EffectName = TEXT("넓은 베기");
                    Tier.TripodEffects[1].Description = TEXT("베기의 영향 범위가 40% 증가합니다.");

                    Tier.TripodEffects[2].EffectName = TEXT("빠른 베기");
                    Tier.TripodEffects[2].Description = TEXT("베기의 쿨다운이 15% 감소합니다.");
                }
                else if (TierIndex == 1) // 2단계 트라이포드
                {
                    Tier.TripodEffects[0].EffectName = TEXT("이중 베기");
                    Tier.TripodEffects[0].Description = TEXT("베기가 한 번 더 발동됩니다.");

                    Tier.TripodEffects[1].EffectName = TEXT("관통 베기");
                    Tier.TripodEffects[1].Description = TEXT("베기가 적의 방어력을 20% 무시합니다.");

                    Tier.TripodEffects[2].EffectName = TEXT("빠른 시전");
                    Tier.TripodEffects[2].Description = TEXT("시전 속도가 30% 증가합니다.");
                }
                else if (TierIndex == 2) // 3단계 트라이포드
                {
                    Tier.TripodEffects[0].EffectName = TEXT("파괴적 베기");
                    Tier.TripodEffects[0].Description = TEXT("베기가 적중하면 주변에 2차 폭발을 일으킵니다.");

                    Tier.TripodEffects[1].EffectName = TEXT("전기 베기");
                    Tier.TripodEffects[1].Description = TEXT("베기에 전기 속성이 추가되어 5초간 도트 데미지를 입힙니다.");

                    Tier.TripodEffects[2].EffectName = TEXT("마법 베기");
                    Tier.TripodEffects[2].Description = TEXT("베기가 투사체를 발사합니다.");
                }
            }
            else if (Skill.SkillName.Equals(TEXT("발차기")))
            {
                if (TierIndex == 0) // 1단계 트라이포드
                {
                    Tier.TripodEffects[0].EffectName = TEXT("강화된 발차기");
                    Tier.TripodEffects[0].Description = TEXT("발차기의 데미지가 25% 증가합니다.");

                    Tier.TripodEffects[1].EffectName = TEXT("넓은 발차기");
                    Tier.TripodEffects[1].Description = TEXT("발차기의 영향 범위가 40% 증가합니다.");

                    Tier.TripodEffects[2].EffectName = TEXT("빠른 발차기");
                    Tier.TripodEffects[2].Description = TEXT("발차기의 쿨다운이 15% 감소합니다.");
                }
                else if (TierIndex == 1) // 2단계 트라이포드
                {
                    Tier.TripodEffects[0].EffectName = TEXT("이중 발차기");
                    Tier.TripodEffects[0].Description = TEXT("발차기가 한 번 더 발동됩니다.");

                    Tier.TripodEffects[1].EffectName = TEXT("관통 발차기");
                    Tier.TripodEffects[1].Description = TEXT("발차기가 적의 방어력을 20% 무시합니다.");

                    Tier.TripodEffects[2].EffectName = TEXT("빠른 시전");
                    Tier.TripodEffects[2].Description = TEXT("시전 속도가 30% 증가합니다.");
                }
                else if (TierIndex == 2) // 3단계 트라이포드
                {
                    Tier.TripodEffects[0].EffectName = TEXT("파괴적 발차기");
                    Tier.TripodEffects[0].Description = TEXT("발차기가 적중하면 주변에 2차 폭발을 일으킵니다.");

                    Tier.TripodEffects[1].EffectName = TEXT("화염 발차기");
                    Tier.TripodEffects[1].Description = TEXT("발차기에 화염 속성이 추가되어 5초간 도트 데미지를 입힙니다.");

                    Tier.TripodEffects[2].EffectName = TEXT("마법 발차기");
                    Tier.TripodEffects[2].Description = TEXT("발차기가 투사체를 발사합니다.");
                }
            }
            else if (Skill.SkillName.Equals(TEXT("회전베기")))
            {
                if (TierIndex == 0) // 1단계 트라이포드
                {
                    Tier.TripodEffects[0].EffectName = TEXT("강화된 회전베기");
                    Tier.TripodEffects[0].Description = TEXT("회전베기의 데미지가 25% 증가합니다.");

                    Tier.TripodEffects[1].EffectName = TEXT("넓은 회전베기");
                    Tier.TripodEffects[1].Description = TEXT("회전베기의 영향 범위가 40% 증가합니다.");

                    Tier.TripodEffects[2].EffectName = TEXT("빠른 회전베기");
                    Tier.TripodEffects[2].Description = TEXT("회전베기의 쿨다운이 15% 감소합니다.");
                }
                else if (TierIndex == 1) // 2단계 트라이포드
                {
                    Tier.TripodEffects[0].EffectName = TEXT("이중 회전베기");
                    Tier.TripodEffects[0].Description = TEXT("회전베기가 한 번 더 발동됩니다.");

                    Tier.TripodEffects[1].EffectName = TEXT("관통 회전베기");
                    Tier.TripodEffects[1].Description = TEXT("회전베기가 적의 방어력을 20% 무시합니다.");

                    Tier.TripodEffects[2].EffectName = TEXT("빠른 시전");
                    Tier.TripodEffects[2].Description = TEXT("시전 속도가 30% 증가합니다.");
                }
                else if (TierIndex == 2) // 3단계 트라이포드
                {
                    Tier.TripodEffects[0].EffectName = TEXT("파괴적 회전베기");
                    Tier.TripodEffects[0].Description = TEXT("회전베기가 적중하면 주변에 2차 폭발을 일으킵니다.");

                    Tier.TripodEffects[1].EffectName = TEXT("바람 회전베기");
                    Tier.TripodEffects[1].Description = TEXT("회전베기에 바람 속성이 추가되어 5초간 도트 데미지를 입힙니다.");

                    Tier.TripodEffects[2].EffectName = TEXT("마법 회전베기");
                    Tier.TripodEffects[2].Description = TEXT("회전베기가 투사체를 발사합니다.");
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
        SkillCooldownRemaining[SkillIndex] = 0.0f;
        
        // 쿨다운 텍스트 초기화
        if (SkillWidget)
        {
            SkillWidget->ResetSkillCooldownText(SkillIndex);
        }
        
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
        SkillCooldownRemaining[SkillIndex] = CooldownTime;

        // 타이머 설정
        GetWorld()->GetTimerManager().SetTimer(
            SkillCooldownTimers[SkillIndex],
            FTimerDelegate::CreateUObject(this, &AMostArkPlayer::OnSkillCooldownComplete, SkillIndex),
            CooldownTime,
            false);

        UE_LOG(LogTemp, Display, TEXT("스킬 %s의 쿨다운 시작 (%.1f초)"), *Skills[SkillIndex].SkillName, CooldownTime);
    }
}

// 쿨다운 텍스트 업데이트 함수
void AMostArkPlayer::UpdateCooldownTexts()
{
    if (!SkillWidget)
        return;
        
    for (int32 i = 0; i < Skills.Num(); ++i)
    {
        if (!bSkillAvailable[i])
        {
            // 남은 시간 계산
            float RemainingTime = 0.0f;
            if (GetWorld()->GetTimerManager().IsTimerActive(SkillCooldownTimers[i]))
            {
                RemainingTime = GetWorld()->GetTimerManager().GetTimerRemaining(SkillCooldownTimers[i]);
                SkillCooldownRemaining[i] = RemainingTime;
            }
            
            // 위젯 텍스트 업데이트
            SkillWidget->UpdateSkillCooldownText(i, RemainingTime);
        }
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

void AMostArkPlayer::FireProjectile(float baseDamage, float attackMultiplier)
{
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    FVector ProjectileStart = GetActorLocation();
    FRotator ProjectileRotation = GetActorRotation();

    ABaseProjectile* Projectile = GetWorld()->SpawnActor<ABaseProjectile>(BasicProjectile, ProjectileStart, ProjectileRotation, SpawnParams);
 
    if (Projectile)
    {
        FAttackInfo Info;
        Info.Damage = baseDamage * attackMultiplier * BaseMultiplier;
        Info.DamageType = UDamageType::StaticClass();
        Info.InstigatorActor = this;

        Projectile->SetAttackInfo(Info);
    }
}

// 콜리전 오버랩 구현
void AMostArkPlayer::OnSwordCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 이미 현재 애니메이션에서 데미지가 적용되었는지 확인
    if (bDamageAppliedForSwordAnimation)
    {
        return; // 이미 데미지가 적용되었으면 추가 데미지 적용하지 않음
    }
    
    if (OtherActor && OtherActor != this)
    {
        // 현재 활성화된 스킬의 데미지 계산
        float DamageAmount = 50.0f; // 기본값
        
        // 베기(0) 또는 회전베기(2) 스킬 사용 중일 때
        if (ActiveSkillIndex == 0 || ActiveSkillIndex == 2)
        {
            DamageAmount = CalculateSkillDamage(ActiveSkillIndex);
        }
        
        UGameplayStatics::ApplyDamage(OtherActor, DamageAmount, GetController(), this, nullptr);

        if (AttackVFX)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), AttackVFX, OtherActor->GetActorLocation(), FRotator::ZeroRotator, FVector(3,3,3));
        }
        
        // 데미지가 적용되었음을 표시
        bDamageAppliedForSwordAnimation = true;
    }
}

void AMostArkPlayer::OnKickCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 이미 현재 애니메이션에서 데미지가 적용되었는지 확인
    if (bDamageAppliedForKickAnimation)
    {
        return; // 이미 데미지가 적용되었으면 추가 데미지 적용하지 않음
    }
    
    if (OtherActor && OtherActor != this)
    {
        // 발차기(1) 스킬 사용 중일 때
        float DamageAmount = 30.0f; // 기본값
        
        if (ActiveSkillIndex == 1)
        {
            DamageAmount = CalculateSkillDamage(ActiveSkillIndex);
        }
        
        UGameplayStatics::ApplyDamage(OtherActor, DamageAmount, GetController(), this, nullptr);
        
        // 데미지가 적용되었음을 표시
        bDamageAppliedForKickAnimation = true;
    }
}
