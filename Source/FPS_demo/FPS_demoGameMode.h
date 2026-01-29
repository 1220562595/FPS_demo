// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "FPS_demoGameMode.generated.h"

class ABossAIController;
class ABossCharacter;

UCLASS(minimalapi)
class AFPS_demoGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AFPS_demoGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnMatchStateSet() override;

	virtual void PlayerEliminated(class AFPS_demoCharacter* ElimmedCharacter, class AFPS_demoPlayerController* VictimController, AFPS_demoPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

	float CountdownTime = 0.0f;
	float LevelStartingTime = 0.0f;

	UPROPERTY(EditAnywhere)
	float MatchTime = 120.0f;

	int32 ReadyTravelCount = 0;

	UFUNCTION()
	void AddReadyTravelPlayer(AFPS_demoPlayerController* PC);
};



