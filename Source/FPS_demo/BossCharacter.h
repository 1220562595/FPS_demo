// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"

#include "BossCharacter.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;
class UFPS_demoAbilitySystemComponent;

UCLASS()
class FPS_DEMO_API ABossCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABossCharacter();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void PossessedBy(AController* NewController) override;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	virtual void Tick(float DeltaTime) override;

	// 保存Character减速前的原始移动速度
	float OriginalMaxWalkSpeed = 0.0f;
	// 标记是否已应用减速
	bool bHasAppliedSlow = false;

	UPROPERTY(EditAnywhere)
	float MaxHealth = 100.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, ReplicatedUsing = OnRep_Health)
	float Health;

	UFUNCTION()
	void OnRep_Health();

	UPROPERTY(EditAnywhere)
	float AcceptanceRadius{ 500.0f };

	UPROPERTY(EditAnywhere)
	float MinAttackDelay{ 0.1f };

	UPROPERTY(EditAnywhere)
	float MaxAttackDelay{ 0.5f };

	// 死亡动画Montage（在蓝图中指定）
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* DeathAnimMontage;

	UPROPERTY(/*ReplicatedUsing = OnRep_TargetPlayer*/)
	class AFPS_demoCharacter* TargetPlayer;

	/*UFUNCTION()
	void OnRep_TargetPlayer();*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UBossTargetPerceptionComponent* TargetPerception;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void AttackForwardArea(float Damage, float AttackRange, float AttackAngleDeg);

protected:
	virtual void BeginPlay() override;
	void GiveStartupAbilities();

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UFPS_demoAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;
};
