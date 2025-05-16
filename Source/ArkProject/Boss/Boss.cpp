// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss.h"

#include "BossAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "../UI/DamageTextActor.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

// Sets default values
ABoss::ABoss()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCharacterMovement()->bEnablePhysicsInteraction = false;
	GetCharacterMovement()->PushForceFactor = 0.f;
	GetCharacterMovement()->bPushForceUsingZOffset = false;
	GetCharacterMovement()->bScalePushForceToVelocity = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	// 기본 충돌 컴포넌트 초기화
	HitCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("HitCollision"));
	HitCollision->SetupAttachment(GetMesh(), TEXT("HitCollision"));
	HitCollision->SetCollisionProfileName(TEXT("EnemyHit"));

	// 왼손 충돌 컴포넌트 초기화
	LeftHitCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftHitCollision"));
	LeftHitCollision->SetupAttachment(GetMesh(), TEXT("hand_l"));
	LeftHitCollision->SetCollisionProfileName(TEXT("NoCollision"));
	LeftHitCollision->SetBoxExtent(FVector(15.0f, 15.0f, 15.0f));
	LeftHitCollision->SetHiddenInGame(true);

	// 오른손 충돌 컴포넌트 초기화
	RightHitCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightHitCollision"));
	RightHitCollision->SetupAttachment(GetMesh(), TEXT("hand_r"));
	RightHitCollision->SetCollisionProfileName(TEXT("NoCollision"));
	RightHitCollision->SetBoxExtent(FVector(15.0f, 15.0f, 15.0f));
	RightHitCollision->SetHiddenInGame(true);

	// 꼬리 충돌 컴포넌트 초기화
	BackHitCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BackHitCollision"));
	BackHitCollision->SetupAttachment(GetMesh(), TEXT("TailSocket"));
	BackHitCollision->SetCollisionProfileName(TEXT("NoCollision"));
	BackHitCollision->SetBoxExtent(FVector(20.0f, 20.0f, 20.0f));
	BackHitCollision->SetHiddenInGame(true);

	// 그라운드 패턴 컴포넌트 초기화
	GroundHitCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("GroundHitCollision"));
	GroundHitCollision->SetupAttachment(GetMesh(), TEXT("GroundSocket"));
	GroundHitCollision->SetCollisionProfileName(TEXT("NoCollision"));
	GroundHitCollision->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
	GroundHitCollision->SetHiddenInGame(true);

	// 밀림 현상 방지
	GetCharacterMovement()->bEnablePhysicsInteraction = false;
	GetCharacterMovement()->PushForceFactor = 0;
}

// Called when the game starts or when spawned
void ABoss::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;

	// 충돌 이벤트 바인딩
	LeftHitCollision->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnLeftHandOverlapBegin);
	RightHitCollision->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnRightHandOverlapBegin);
	BackHitCollision->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnTailOverlapBegin);
	GroundHitCollision->OnComponentBeginOverlap.AddDynamic(this, &ABoss::OnGroundOverlapBegin);

	// 초기에는 모든 공격 충돌 비활성화
	ActivateLeftHandAttack(false);
	ActivateRightHandAttack(false);
	ActivateTailAttack(false);
	ActivateGroundAttack(false);
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
	if (ActualDamage > 0.0f && DamageTextActorClass)
	{
			// 데미지 위치 (머리 위쪽으로 약간 올림)
			FVector DamageLocation = this->GetActorLocation() + FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 1.2f);
			
			// 크리티컬 여부 (임시: 100 이상의 데미지는 크리티컬로 표시)
			bool bIsCritical = ActualDamage >= 100.0f;
			
			// 월드에 데미지 텍스트 액터 생성
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			
			// 데미지 텍스트 액터 생성
			ADamageTextActor* DamageActor = GetWorld()->SpawnActor<ADamageTextActor>(
				DamageTextActorClass,
				DamageLocation,
				FRotator::ZeroRotator,
				SpawnParams
			);
			
			// 데미지 텍스트 초기화
			if (DamageActor)
			{
				DamageActor->Initialize(ActualDamage, bIsCritical);
			}
		
	}
	
	// 데미지 이벤트 호출
	OnDamaged(ActualDamage, DamageCauser);
	
	// 체력이 0이 되면 사망 처리
	if (CurrentHealth <= 0.0f)
	{
		// 사망 이벤트 호출
		OnDeath();
		
		// 사망 애니메이션 재생
		UBossAnimInstance* AnimInstance = Cast<UBossAnimInstance>(GetMesh()->GetAnimInstance());
		if (AnimInstance)
		{
			AnimInstance->PlayDeathMontage();
			
			// 충돌 비활성화
			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			
			// 모든 공격 충돌 비활성화
			ActivateLeftHandAttack(false);
			ActivateRightHandAttack(false);
			ActivateTailAttack(false);
			ActivateGroundAttack(false);
			
			// 5초 후 액터 파괴
			FTimerHandle DestroyTimerHandle;
			GetWorldTimerManager().SetTimer(DestroyTimerHandle, [this]()
			{
				this->Destroy();
			}, 5.0f, false);
		}
	}
	
	return ActualDamage;
}

// 왼손 충돌 이벤트 처리
void ABoss::OnLeftHandOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 자기 자신과의 충돌은 무시
	if (OtherActor == this)
	{
		return;
	}

	// 충돌한 액터에 데미지 적용
	ApplyDamageToTarget(OtherActor, LeftAttackDamage);

	// 델리게이트 브로드캐스트
	OnLeftHandHit.Broadcast(OtherActor, SweepResult.BoneName);
}

// 오른손 충돌 이벤트 처리
void ABoss::OnRightHandOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 자기 자신과의 충돌은 무시
	if (OtherActor == this)
	{
		return;
	}

	// 충돌한 액터에 데미지 적용
	ApplyDamageToTarget(OtherActor, RightAttackDamage);

	// 델리게이트 브로드캐스트
	OnRightHandHit.Broadcast(OtherActor, SweepResult.BoneName);
}

// 꼬리 충돌 이벤트 처리
void ABoss::OnTailOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 자기 자신과의 충돌은 무시
	if (OtherActor == this)
	{
		return;
	}

	// 충돌한 액터에 데미지 적용
	ApplyDamageToTarget(OtherActor, TailAttackDamage);

	// 델리게이트 브로드캐스트
	OnTailHit.Broadcast(OtherActor, SweepResult.BoneName);
}

void ABoss::OnGroundOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 자기 자신과의 충돌은 무시
	if (OtherActor == this)
	{
		return;
	}

	// 충돌한 액터에 데미지 적용
	ApplyDamageToTarget(OtherActor, GroundAttackDamage);

	// 델리게이트 브로드캐스트
	OnGroundHit.Broadcast(OtherActor, SweepResult.BoneName);
}

// 왼손 공격 활성화/비활성화
void ABoss::ActivateLeftHandAttack(bool bActivate)
{
	LeftHitCollision->SetCollisionEnabled(bActivate ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	LeftHitCollision->SetCollisionResponseToAllChannels(bActivate ? ECR_Overlap : ECR_Ignore);
}

// 오른손 공격 활성화/비활성화
void ABoss::ActivateRightHandAttack(bool bActivate)
{
	RightHitCollision->SetCollisionEnabled(bActivate ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	RightHitCollision->SetCollisionResponseToAllChannels(bActivate ? ECR_Overlap : ECR_Ignore);
}

// 꼬리 공격 활성화/비활성화
void ABoss::ActivateTailAttack(bool bActivate)
{
	BackHitCollision->SetCollisionEnabled(bActivate ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	BackHitCollision->SetCollisionResponseToAllChannels(bActivate ? ECR_Overlap : ECR_Ignore);
}

void ABoss::ActivateGroundAttack(bool bActivate)
{
	GroundHitCollision->SetCollisionEnabled(bActivate ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	GroundHitCollision->SetCollisionResponseToAllChannels(bActivate ? ECR_Overlap : ECR_Ignore);
}

// 데미지 적용 함수
void ABoss::ApplyDamageToTarget(AActor* TargetActor, float DamageAmount)
{
	if (TargetActor && DamageAmount > 0.0f)
	{
		// 데미지 적용
		UGameplayStatics::ApplyDamage(
			TargetActor,
			DamageAmount,
			GetController(),
			this,
			UDamageType::StaticClass()
		);

		FVector ImpactLocation;
		// 캐릭터만 타격 이펙트 생성
		ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor);
		if (TargetCharacter)
		{
			// 캐릭터의 경우 캐릭터 중앙 위치에 생성
			ImpactLocation = TargetCharacter->GetMesh()->GetComponentLocation();
		}
		
		// 데미지 효과 VFX 생성
		SpawnDamageImpactVFX(ImpactLocation);
	}
}

// 그라운드 공격 VFX 스폰 함수
UNiagaraComponent* ABoss::SpawnGroundAttackVFX(const FVector& Location)
{
	// VFX 시스템이 설정되어 있는지 확인
	if (!GroundAttackVFX)
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss - GroundAttackVFX가 설정되지 않았습니다."));
		return nullptr;
	}
	
	FVector SpawnLocation = GetActorLocation();
	UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		GroundAttackVFX,
		SpawnLocation,
		FRotator::ZeroRotator,
		GroundAttackVFXScale,
		true,
		true,
		ENCPoolMethod::AutoRelease);
	
		if (NiagaraComp)
		{
			UE_LOG(LogTemp, Log, TEXT("Boss - 그라운드 공격 시작 VFX 생성 성공: %s"), *SpawnLocation.ToString());
		}
		return NiagaraComp;
}

// 그라운드 공격 종료 VFX 스폰 함수
UNiagaraComponent* ABoss::SpawnGroundAttackFinishVFX(const FVector& Location)
{
	// VFX 시스템이 설정되어 있는지 확인
	if (!GroundAttackFinishVFX)
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss - GroundAttackFinishVFX가 설정되지 않았습니다."));
		return nullptr;
	}
	
	FVector SpawnLocation = GetActorLocation();
	UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		GroundAttackFinishVFX,
		SpawnLocation,
		FRotator::ZeroRotator,
		GroundAttackFinishVFXScale,
		true,
		true,
		ENCPoolMethod::AutoRelease
	);

	if (NiagaraComp)
	{
		UE_LOG(LogTemp, Log, TEXT("Boss - 그라운드 공격 종료 VFX 생성 성공: %s"), *SpawnLocation.ToString());
	}
	return NiagaraComp;
}

// 데미지 효과 VFX 스폰 함수
UNiagaraComponent* ABoss::SpawnDamageImpactVFX(const FVector& Location)
{
	// VFX 시스템이 설정되어 있는지 확인
	if (!DamageImpactVFX)
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss - DamageImpactVFX가 설정되지 않았습니다."));
		return nullptr;
	}

	// 데미지 위치에 VFX 생성 (레이캐스트 없이 직접 위치에 생성)
	FVector SpawnLocation = Location + DamageImpactVFXOffset;
	UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		DamageImpactVFX,
		SpawnLocation,
		FRotator::ZeroRotator,
		DamageImpactVFXScale,
		true,
		true,
		ENCPoolMethod::AutoRelease
	);

	if (NiagaraComp)
	{
		UE_LOG(LogTemp, Log, TEXT("Boss - 데미지 효과 VFX 생성 성공: %s"), *SpawnLocation.ToString());
	}
	return NiagaraComp;
}
