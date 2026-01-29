// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Attack.h"
#include "BossCharacter.h"
#include "AIController.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Attack");
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (OwnerComp.GetAIOwner() == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	ABossCharacter* Boss = Cast<ABossCharacter>(OwnerComp.GetAIOwner()->GetPawn());

	if (Boss == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	Boss->AttackForwardArea(30.f, 500.f, 90.f);

	return EBTNodeResult::Succeeded;
}
