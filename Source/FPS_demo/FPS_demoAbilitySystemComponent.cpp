// Fill out your copyright notice in the Description page of Project Settings.


#include "FPS_demoAbilitySystemComponent.h"
#include "Tags.h"

UFPS_demoAbilitySystemComponent::UFPS_demoAbilitySystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFPS_demoAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UFPS_demoAbilitySystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UFPS_demoAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);

	HandleAutoActivatedAbility(AbilitySpec);
}

void UFPS_demoAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	FScopedAbilityListLock ActiveScopeLock(*this);
	for (const auto& AbilitySpec : GetActivatableAbilities())
	{
		HandleAutoActivatedAbility(AbilitySpec);
	}
}

void UFPS_demoAbilitySystemComponent::HandleAutoActivatedAbility(const FGameplayAbilitySpec& AbilitySpec)
{
	if (!IsValid(AbilitySpec.Ability)) return;

	for (const FGameplayTag& Tag : AbilitySpec.Ability->AbilityTags)
	{
		if (Tag.MatchesTagExact(Tags::Abilities::ActivateOnGiven))
		{
			TryActivateAbility(AbilitySpec.Handle);
			return;
		}
	}
}