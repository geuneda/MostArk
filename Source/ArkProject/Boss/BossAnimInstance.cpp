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


void UBossAnimInstance::PlayLeftMoveAttackMontage() const
{
	if (Boss)
	{
		Boss->PlayAnimMontage(LeftMoveAttackMontage, 1.0f);
	}
}

void UBossAnimInstance::PlayLeftAttackMontage() const
{
	if (Boss)
	{
		Boss->PlayAnimMontage(LeftAttackMontage, 1.0f);
	}
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

void UBossAnimInstance::PlayRightMoveAttackMontage() const
{
	if (Boss)
	{
		Boss->PlayAnimMontage(RightMoveAttackMontage, 1.0f);
	}
}

void UBossAnimInstance::PlayRightAttackMontage() const
{
	if (Boss)
	{
		Boss->PlayAnimMontage(RightAttackMontage, 1.0f);
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

void UBossAnimInstance::PlayTailAttackMontage() const
{
	if (Boss)
	{
		Boss->PlayAnimMontage(TailAttackMontage, 1.0f);
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

void UBossAnimInstance::PlayGroundAttackMontage() const
{
	if (Boss)
	{
		FString AttackName = FString::Printf(TEXT("Under"));
		Boss->PlayAnimMontage(GroundAttackMontage, 1.0f, FName(*AttackName));
	}
}

void UBossAnimInstance::StopGroundAttackMontage() const
{
	if (Boss)
	{
		FString AttackName = FString::Printf(TEXT("Off"));
		Boss->PlayAnimMontage(GroundAttackMontage, 1.0f, FName(*AttackName));
	}
}

void UBossAnimInstance::AnimNotify_Hide()
{
	if (Boss)
	{
		Boss->GetMesh()->SetVisibility(false);
	}
	else
	{
		InitializeBoss();
	}
}

void UBossAnimInstance::AnimNotify_Show()
{
	if (Boss)
	{
		Boss->GetMesh()->SetVisibility(true);
	}
	else
	{
		InitializeBoss();
	}
}

void UBossAnimInstance::AnimNotify_GroundAttackStart()
{
	if (Boss)
	{
		Boss->ActivateGroundAttack(true);
	}
	else
	{
		InitializeBoss();
	}
}

void UBossAnimInstance::AnimNotify_GroundAttackEnd()
{
	if (Boss)
	{
		Boss->ActivateGroundAttack(false);
	}
	else
	{
		InitializeBoss();
	}
}

