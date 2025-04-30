// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"

ABaseProjectile::ABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// 충돌체
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->InitSphereRadius(30.f);
	CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
	RootComponent = CollisionComp;

	// 파티클
	ParticleComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleComp"));
	ParticleComp->SetupAttachment(CollisionComp);

	// 움직임
	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMovementComp->InitialSpeed = ProjectileSpeed;
	ProjectileMovementComp->MaxSpeed = ProjectileSpeed;

	ProjectileMovementComp->SetUpdatedComponent(RootComponent);
}

// Called when the game starts or when spawned
void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABaseProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseProjectile::SetAttackInfo(const FAttackInfo& Info)
{
	AttackInfo = Info;
}

void ABaseProjectile::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (OtherActor)
	{
		UGameplayStatics::ApplyDamage(OtherActor, AttackInfo.Damage, AttackInfo.InstigatorActor ? AttackInfo.InstigatorActor->GetInstigatorController() : nullptr, AttackInfo.InstigatorActor, AttackInfo.DamageType);
	}

	// 폭발 이펙트 소환
	if (ImpactParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, GetActorLocation(), GetActorRotation());
	}

	this->Destroy();
}

void ABaseProjectile::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == TEXT("ProjectileSpeed"))
	{
		ProjectileMovementComp->InitialSpeed = ProjectileSpeed;
		ProjectileMovementComp->MaxSpeed = ProjectileSpeed;
	}
}
