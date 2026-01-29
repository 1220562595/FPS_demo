// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "RewardReceiverInterface.h"

#include "FPS_demoPlayerState.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FPlayerScoreData
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DamageToPlayers = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DamageToBoss = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 KillPlayers = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 KillBosses = 0;

	float Cal();
};

UCLASS()
class FPS_DEMO_API AFPS_demoPlayerState : public APlayerState, public IRewardReceiverInterface
{
	GENERATED_BODY()
	
public:
	AFPS_demoPlayerState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_BonusMaxHealth)
	int32 BonusMaxHealth = 0;

	UFUNCTION()
	void OnRep_BonusMaxHealth();

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_BonusDamage)
	float BonusDamage = 0.0f;

	UFUNCTION()
	void OnRep_BonusDamage();

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_BonusMoveSpeed)
	float BonusMoveSpeed = 0.0f;

	UFUNCTION()
	void OnRep_BonusMoveSpeed();

	virtual void ReceiveKillReward_Implementation() override;

	UPROPERTY(ReplicatedUsing = OnRep_ScoreData)
	FPlayerScoreData ScoreData;

	UFUNCTION()
	void OnRep_ScoreData();

	void AddDamageToPlayer(float Damage);
	void AddDamageToBoss(float Damage);
	void AddKillPlayer();
	void AddKillBoss();

	UPROPERTY(ReplicatedUsing = OnRep_ranking)
	int ranking = 0;

	UFUNCTION()
	void OnRep_ranking();
};
