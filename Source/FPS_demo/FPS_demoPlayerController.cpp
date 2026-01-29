// Copyright Epic Games, Inc. All Rights Reserved.


#include "FPS_demoPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "PlayerHUD.h"
#include "CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include <Net/UnrealNetwork.h>
#include "FPS_demoGameMode.h"
#include "MatchEnd.h"
#include "FPS_demoPlayerState.h"
#include <Kismet/GameplayStatics.h>
#include "FPS_demoGameState.h"

void AFPS_demoPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(InputMappingContext, 0);
	}

	PlayerHUD = Cast<APlayerHUD>(GetHUD());

	if (HasAuthority())
	{
		if (AFPS_demoGameMode* GM = Cast<AFPS_demoGameMode>(UGameplayStatics::GetGameMode(this)))
		{
			MatchTime = GM->MatchTime;
		}
	}
}

void AFPS_demoPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPS_demoPlayerController, MatchState);
	DOREPLIFETIME(AFPS_demoPlayerController, MatchTime);
}

void AFPS_demoPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AFPS_demoPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();

	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.0f;
	}
}

void AFPS_demoPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD &&
		PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->HealthBar &&
		PlayerHUD->CharacterOverlay->HealthText;
	if (bHUDValid)
	{
		float HealthPercent = Health / MaxHealth;
		PlayerHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		PlayerHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void AFPS_demoPlayerController::SetHUDMatchCountdown(float MatchCountdownTime)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD &&
		PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->MatchCountdownText;
	if (bHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(MatchCountdownTime / 60.0f);
		int32 Seconds = MatchCountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PlayerHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void AFPS_demoPlayerController::SetHUDTime()
{
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());
	if (CountdownInt != SecondsLeft)
	{
		SetHUDMatchCountdown(MatchTime - GetServerTime());
		CountdownInt = SecondsLeft;
	}

}

void AFPS_demoPlayerController::SetHUDMatchEnd()
{
	AddHUDMatchEndToViewport();
}

void AFPS_demoPlayerController::AddHUDMatchEndToViewport()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	if (AFPS_demoGameState* GS = Cast<AFPS_demoGameState>(UGameplayStatics::GetGameState(this)))
	{
		GS->GenerateRankList();
	}
	PlayerHUD->AddMatchEnd();

	bool bHUDValid = PlayerHUD &&
		PlayerHUD->MatchEnd &&
		PlayerHUD->MatchEnd->Ranking;
	if (!bHUDValid) return;

	if (AFPS_demoPlayerState* FPS_demoPlayerState = GetPlayerState<AFPS_demoPlayerState>())
	{
		FString TempRanking = FString::Printf(TEXT("%d"), FPS_demoPlayerState->ranking);
		PlayerHUD->MatchEnd->Ranking->SetText(FText::FromString(TempRanking));
	}
	PlayerHUD->MatchEnd->AddToViewport();
}

void AFPS_demoPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void AFPS_demoPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + RoundTripTime * 0.5f;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float AFPS_demoPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AFPS_demoPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if (MatchState == MatchState::LeavingMap)
	{
		PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
		if (PlayerHUD)
		{
			SetHUDMatchEnd();
		}
	}
}

void AFPS_demoPlayerController::ServerReadyToTravel_Implementation()
{
	if (AFPS_demoGameMode* GM = GetWorld()->GetAuthGameMode<AFPS_demoGameMode>())
		GM->AddReadyTravelPlayer(this);
}

void AFPS_demoPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
		if (PlayerHUD)
		{
			
		}
	}
}
