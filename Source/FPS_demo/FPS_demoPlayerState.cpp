// Fill out your copyright notice in the Description page of Project Settings.


#include "FPS_demoPlayerState.h"
#include <Net/UnrealNetwork.h>
#include "FPS_demoCharacter.h"
#include "FPS_demoPlayerController.h"

AFPS_demoPlayerState::AFPS_demoPlayerState()
{
	
}

void AFPS_demoPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPS_demoPlayerState, BonusMaxHealth);
	DOREPLIFETIME(AFPS_demoPlayerState, BonusMoveSpeed);
	DOREPLIFETIME(AFPS_demoPlayerState, BonusDamage);
	DOREPLIFETIME(AFPS_demoPlayerState, ranking);
}

void AFPS_demoPlayerState::ReceiveKillReward_Implementation()
{
	if (!HasAuthority()) return;

	const int32 Roll = FMath::RandRange(0, 0);

	switch (Roll)
	{
	case 0: BonusMaxHealth += 10; break;
	case 1: BonusDamage += 5.f; break;
	case 2: BonusMoveSpeed += 30.f; break;
	}

	AFPS_demoCharacter* Character =
		Cast<AFPS_demoCharacter>(GetPawn());

	if (Character)
	{
		
		Character->ApplyPlayerStateBonuses();
	}
}

void AFPS_demoPlayerState::OnRep_BonusMaxHealth()
{
	AFPS_demoCharacter* Character = Cast<AFPS_demoCharacter>(GetPawn());
	if (Character)
	{
		/*GEngine->AddOnScreenDebugMessage(
			-1, 3.0f, FColor::Green,
			FString::Printf(TEXT("BonusMaxHealth: %.1f"), BonusMaxHealth)
		);*/
		Character->ApplyPlayerStateBonuses();
	}
}

void AFPS_demoPlayerState::OnRep_BonusDamage()
{
	AFPS_demoCharacter* Character = Cast<AFPS_demoCharacter>(GetPawn());
	if (Character)
	{
		Character->ApplyPlayerStateBonuses();
	}
}

void AFPS_demoPlayerState::OnRep_BonusMoveSpeed()
{
	AFPS_demoCharacter* Character = Cast<AFPS_demoCharacter>(GetPawn());
	if (Character)
	{
		Character->ApplyPlayerStateBonuses();
	}
}

void AFPS_demoPlayerState::AddDamageToPlayer(float Damage)
{
	if (!HasAuthority()) return;
	ScoreData.DamageToPlayers += Damage;
	/*GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
		FString::Printf(TEXT("DamageToPlayers=%.1f"), ScoreData.DamageToPlayers));*/
}

void AFPS_demoPlayerState::AddDamageToBoss(float Damage)
{
	if (!HasAuthority()) return;
	ScoreData.DamageToBoss += Damage;
	/*GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
		FString::Printf(TEXT("DamageToBoss=%.1f"), ScoreData.DamageToBoss));*/
}

void AFPS_demoPlayerState::AddKillPlayer()
{
	if (!HasAuthority()) return;
	ScoreData.KillPlayers++;
	/*GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
		FString::Printf(TEXT("KillPlayers=%d"), ScoreData.KillPlayers));*/
}

void AFPS_demoPlayerState::AddKillBoss()
{
	if (!HasAuthority()) return;
	ScoreData.KillBosses++;
	/*GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
		FString::Printf(TEXT("KillBosses=%d"), ScoreData.KillBosses));*/
}

void AFPS_demoPlayerState::OnRep_ranking()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
		FString::Printf(TEXT("OnRep_ranking")));
	if (AFPS_demoPlayerController* FPS_demoPlayerController = Cast<AFPS_demoPlayerController>(GetPlayerController()))
	{
		FPS_demoPlayerController->AddHUDMatchEndToViewport();
	}
}

void AFPS_demoPlayerState::OnRep_ScoreData()
{
	
}

float FPlayerScoreData::Cal()
{
	return DamageToPlayers * 2 + DamageToBoss * 1 + KillPlayers * 50 + KillBosses * 200;
}
