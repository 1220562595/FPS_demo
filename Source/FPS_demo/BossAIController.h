// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BossAIController.generated.h"

class UBehaviorTree;
class ABossCharacter;

/**
 * 
 */
UCLASS()
class FPS_DEMO_API ABossAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	ABossCharacter* OwnerCharacter;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	UBehaviorTree* AIBehaviorTree;

	UPROPERTY()
	UBlackboardComponent* BlackboardComponent;
};
