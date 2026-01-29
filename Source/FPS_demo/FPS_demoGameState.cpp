// Fill out your copyright notice in the Description page of Project Settings.


#include "FPS_demoGameState.h"
#include "FPS_demoPlayerState.h"

void AFPS_demoGameState::GenerateRankList() const
{
    if (!HasAuthority()) return;

    TArray<FRankEntry> Work;

    for (APlayerState* PS : PlayerArray)
    {
        if (!PS) continue;
        if (AFPS_demoPlayerState* FPS_demoPlayerState = Cast<AFPS_demoPlayerState>(PS))
        {
            float S = FPS_demoPlayerState->ScoreData.Cal();
            Work.Add({ 0, S, FPS_demoPlayerState });
        }
    }

    Work.Sort([](const FRankEntry& A, const FRankEntry& B)
        {
            return A.Score > B.Score;
        });

    for (int i = 0; i < Work.Num(); ++i)
    {
        if (i == 0)
            Work[i].Rank = 1;               
        else
            Work[i].Rank = (Work[i].Score == Work[i - 1].Score)
            ? Work[i - 1].Rank            
            : i + 1;       

        if (Work[i].PlayerState)
        {
            Work[i].PlayerState->ranking = Work[i].Rank;
        }
    }
}
