// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPS_demoGameplayAbility.h"
#include <Abilities/Tasks/AbilityTask_WaitGameplayEvent.h>

#include "HitReact.generated.h"

/**
 * 
 */
UCLASS()
class FPS_DEMO_API UHitReact : public UFPS_demoGameplayAbility
{
	GENERATED_BODY()
	
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		const FGameplayEventData* TriggerEventData) override;
	// 重写能力结束函数（用于恢复速度）
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	// 可配置的减速比例
	UPROPERTY(EditAnywhere, Category = "HitReact|Movement")
	float SlowDownRatio = 0.3f;
	
	UFUNCTION()
	void OnWaitEventCompleted(FGameplayEventData Payload);

	// 保存等待事件任务的引用（UPROPERTY防止被GC回收）
	UPROPERTY()
	UAbilityTask_WaitGameplayEvent* WaitHitReactEventTask;

	// 要监听的Gameplay Tag（在编辑器中配置，比如"Gameplay.Event.HitReact"）
	UPROPERTY(EditDefaultsOnly, Category = "HitReact")
	FGameplayTag HitReactEventTag;
};
