// Fill out your copyright notice in the Description page of Project Settings.


#include "BossAnimInstance.h"
#include "Boss.h"

void UBossAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	Boss = Cast<ABoss>(TryGetPawnOwner());
}

void UBossAnimInstance::AnimNotify_LeftAttackStart()
{
	if (Boss)
	{
		Boss->ActivateLeftHandAttack(true);
	}
}

void UBossAnimInstance::AnimNotify_LeftAttackEnd()
{
	if (Boss)
	{
		Boss->ActivateLeftHandAttack(false);
	}
}

void UBossAnimInstance::AnimNotify_RightAttackStart()
{
	if (Boss)
	{
		Boss->ActivateRightHandAttack(true);
	}
}

void UBossAnimInstance::AnimNotify_RightAttackEnd()
{
	if (Boss)
	{
		Boss->ActivateRightHandAttack(false);
	}
}

void UBossAnimInstance::AnimNotify_TailAttackStart()
{
	if (Boss)
	{
		Boss->ActivateTailAttack(true);
	}
}

void UBossAnimInstance::AnimNotify_TailAttackEnd()
{
	if (Boss)
	{
		Boss->ActivateTailAttack(false);
	}
}
