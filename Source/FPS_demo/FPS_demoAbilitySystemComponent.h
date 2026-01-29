// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"

#include "FPS_demoAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class FPS_DEMO_API UFPS_demoAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	UFPS_demoAbilitySystemComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void OnRep_ActivateAbilities() override;

protected:
	virtual void BeginPlay() override;

private:
	void HandleAutoActivatedAbility(const FGameplayAbilitySpec& AbilitySpec);
};
