// Fill out your copyright notice in the Description page of Project Settings.

#include "BossBTTaskBase.h"
#include "../Boss.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BlackboardComponent.h"

ABoss* UBossBTTaskBase::GetBossCharacter(UBehaviorTreeComponent* OwnerComp) const
{
    if (!OwnerComp)
    {
        return nullptr;
    }

    AAIController* Controller = Cast<AAIController>(OwnerComp->GetOwner());
    if (!Controller)
    {
        return nullptr;
    }

    return Cast<ABoss>(Controller->GetPawn());
}

AAIController* UBossBTTaskBase::GetBossController(UBehaviorTreeComponent* OwnerComp) const
{
    if (!OwnerComp)
    {
        return nullptr;
    }

    return Cast<AAIController>(OwnerComp->GetOwner());
}

ACharacter* UBossBTTaskBase::GetPlayerCharacter(UBehaviorTreeComponent* OwnerComp) const
{
    return Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}
