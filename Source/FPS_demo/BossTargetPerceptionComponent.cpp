// Fill out your copyright notice in the Description page of Project Settings.


#include "BossTargetPerceptionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "FPS_demoCharacter.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

UBossTargetPerceptionComponent::UBossTargetPerceptionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicated(false);
}

void UBossTargetPerceptionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(
			SearchTimerHandle,
			this,
			&UBossTargetPerceptionComponent::UpdateTarget,
			SearchInterval,
			true
		);
	}
}

void UBossTargetPerceptionComponent::UpdateTarget()
{
	TArray<AFPS_demoCharacter*> Targets;
	FindValidTargets(Targets);

	if (Targets.IsEmpty())
	{
		CurrentTarget = nullptr;
		return;
	}

	switch (SearchMode)
	{
	case EBossSearchMode::Closest:
		CurrentTarget = FindClosest(Targets);
		break;

	case EBossSearchMode::LowestHealth:
		CurrentTarget = FindLowestHealth(Targets);
		break;
	}
}

void UBossTargetPerceptionComponent::FindValidTargets(TArray<AFPS_demoCharacter*>& OutTargets) const
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("Player"), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		AFPS_demoCharacter* Player = Cast<AFPS_demoCharacter>(Actor);
		if (!Player) continue;

		if (Player->Health <= 0.f) continue;

		OutTargets.Add(Player);
	}
}

AFPS_demoCharacter* UBossTargetPerceptionComponent::FindClosest(
	const TArray<AFPS_demoCharacter*>& Targets) const
{
	AActor* Owner = GetOwner();

	float MinDistSq = TNumericLimits<float>::Max();
	AFPS_demoCharacter* Result = nullptr;

	for (AFPS_demoCharacter* T : Targets)
	{
		const float DistSq = FVector::DistSquared(
			Owner->GetActorLocation(),
			T->GetActorLocation()
		);

		if (DistSq < MinDistSq)
		{
			MinDistSq = DistSq;
			Result = T;
		}
	}

	return Result;
}

AFPS_demoCharacter* UBossTargetPerceptionComponent::FindLowestHealth(
	const TArray<AFPS_demoCharacter*>& Targets) const
{
	float MinHealth = TNumericLimits<float>::Max();
	AFPS_demoCharacter* Result = nullptr;

	for (AFPS_demoCharacter* T : Targets)
	{
		if (T->Health < MinHealth)
		{
			MinHealth = T->Health;
			Result = T;
		}
	}

	return Result;
}

