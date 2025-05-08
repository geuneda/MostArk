// Fill out your copyright notice in the Description page of Project Settings.


#include "BossAnimInstance.h"
#include "Boss.h"

void UBossAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	InitializeBoss();
}

void UBossAnimInstance::InitializeBoss()
{
	Boss = Cast<ABoss>(TryGetPawnOwner());
}


void UBossAnimInstance::AnimNotify_LeftAttackStart()
{
	if (Boss)
	{
		Boss->ActivateLeftHandAttack(true);
	}
	else
	{
		InitializeBoss();
	}
}

void UBossAnimInstance::AnimNotify_LeftAttackEnd()
{
	if (Boss)
	{
		Boss->ActivateLeftHandAttack(false);
	}
	else
	{
		InitializeBoss();
	}
}

void UBossAnimInstance::AnimNotify_RightAttackStart()
{
	if (Boss)
	{
		Boss->ActivateRightHandAttack(true);
	}
	else
	{
		InitializeBoss();
	}
}

void UBossAnimInstance::AnimNotify_RightAttackEnd()
{
	if (Boss)
	{
		Boss->ActivateRightHandAttack(false);
	}
	else
	{
		InitializeBoss();
	}
}

void UBossAnimInstance::AnimNotify_TailAttackStart()
{
	if (Boss)
	{
		Boss->ActivateTailAttack(true);
	}
	else
	{
		InitializeBoss();
	}
}

void UBossAnimInstance::AnimNotify_TailAttackEnd()
{
	if (Boss)
	{
		Boss->ActivateTailAttack(false);
	}
	else
	{
		InitializeBoss();
	}
}

