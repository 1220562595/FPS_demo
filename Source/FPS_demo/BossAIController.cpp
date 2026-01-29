// Fill out your copyright notice in the Description page of Project Settings.


#include "BossAIController.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BossCharacter.h"
#include "FPS_demoCharacter.h"

void ABossAIController::BeginPlay()
{
	Super::BeginPlay();

	if (AIBehaviorTree)
	{
		RunBehaviorTree(AIBehaviorTree);
	}
	BlackboardComponent = GetBlackboardComponent();
	
}

void ABossAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UBlackboardComponent* BB = GetBlackboardComponent();
	ABossCharacter* Boss = Cast<ABossCharacter>(GetPawn());

	if (!BB || !Boss)
	{
		return;
	}

	AFPS_demoCharacter* Target = Boss->TargetPlayer;

	if (IsValid(Target))
	{
		BB->SetValueAsObject(TEXT("TargetActor"), Target);
		BB->SetValueAsVector(TEXT("TargetLocation"), Target->GetActorLocation());
		SetFocus(Target);
	}
	else
	{
		BB->ClearValue(TEXT("TargetActor"));
		BB->ClearValue(TEXT("TargetLocation"));
		ClearFocus(EAIFocusPriority::Gameplay);
	}
	/*GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
		OwnerCharacter->TargetPlayerLocation.ToString());*/
}
