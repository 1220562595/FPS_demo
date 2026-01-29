// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BossTargetPerceptionComponent.generated.h"

class AFPS_demoCharacter;

UENUM(BlueprintType)
enum class EBossSearchMode : uint8
{
	Closest,
	LowestHealth
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPS_DEMO_API UBossTargetPerceptionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBossTargetPerceptionComponent();

	void UpdateTarget();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search")
	EBossSearchMode SearchMode = EBossSearchMode::Closest;

	UPROPERTY(EditAnywhere, Category = "Search")
	float SearchInterval = 0.5f;

	AFPS_demoCharacter* GetCurrentTarget() const { return CurrentTarget; }

protected:
	virtual void BeginPlay() override;

	void FindValidTargets(TArray<AFPS_demoCharacter*>& OutTargets) const;

	AFPS_demoCharacter* FindClosest(const TArray<AFPS_demoCharacter*>& Targets) const;
	AFPS_demoCharacter* FindLowestHealth(const TArray<AFPS_demoCharacter*>& Targets) const;

	UPROPERTY()
	AFPS_demoCharacter* CurrentTarget = nullptr;

	FTimerHandle SearchTimerHandle;

public:	
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
		
};
