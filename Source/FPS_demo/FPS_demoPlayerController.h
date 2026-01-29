// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPS_demoPlayerController.generated.h"

class UInputMappingContext;

/**
 *
 */
UCLASS()
class FPS_DEMO_API AFPS_demoPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* InputMappingContext;
	
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDMatchCountdown(float MatchCountdownTime);
	void SetHUDTime();
	void SetHUDMatchEnd();

	void AddHUDMatchEndToViewport();

	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.0f;

	UPROPERTY(EditAnywhere)
	float TimeSyncFrequency = 5.0f;

	float TimeSyncRunningTime = 0.0f;

	virtual void ReceivedPlayer() override;

	void OnMatchStateSet(FName State);

	UFUNCTION(Server, Reliable)
	void ServerReadyToTravel();


protected:
	virtual void Tick(float DeltaTime) override;
	virtual float GetServerTime();

private:
	UPROPERTY()
	class APlayerHUD* PlayerHUD;

	UPROPERTY(Replicated)
	float MatchTime;

	uint32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();
};
