// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "FPS_demoGameState.generated.h"

/**
 * 
 */
USTRUCT()
struct FRankEntry
{
    GENERATED_BODY()

public:
    UPROPERTY()
    int32 Rank = 0;

    UPROPERTY()
    float Score = 0.f;

    UPROPERTY()
    class AFPS_demoPlayerState* PlayerState = nullptr;
};

UCLASS()
class FPS_DEMO_API AFPS_demoGameState : public AGameState
{
	GENERATED_BODY()

public:
    UFUNCTION()
    void GenerateRankList() const;
};
