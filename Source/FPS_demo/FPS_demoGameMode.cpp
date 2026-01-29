// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPS_demoGameMode.h"
#include "FPS_demoCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "FPS_demoPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include <EnhancedInputSubsystems.h>
#include "TP_WeaponComponent.h"
#include <EnhancedInputComponent.h>
#include "BossCharacter.h"
#include "BossAIController.h"

AFPS_demoGameMode::AFPS_demoGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

    bDelayedStart = false;
}

void AFPS_demoGameMode::BeginPlay()
{
    Super::BeginPlay();

    LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AFPS_demoGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (MatchState == MatchState::InProgress)
    {
        if (GetWorld()->GetTimeSeconds() - LevelStartingTime >= MatchTime)
        {
            SetMatchState(MatchState::LeavingMap);
        }
    }
}

void AFPS_demoGameMode::OnMatchStateSet()
{
    Super::OnMatchStateSet();

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
    {
        AFPS_demoPlayerController* FPS_demoPlayerController = Cast<AFPS_demoPlayerController>(*It);
        if (FPS_demoPlayerController)
        {
            FPS_demoPlayerController->OnMatchStateSet(MatchState);
        }
    }
}

void AFPS_demoGameMode::PlayerEliminated(AFPS_demoCharacter* ElimmedCharacter, AFPS_demoPlayerController* VictimController, AFPS_demoPlayerController* AttackerController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
}

void AFPS_demoGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
    if (ElimmedController && ElimmedCharacter)
    {
        ElimmedCharacter->Reset();
        TArray<AActor*> PlayerStarts;
        UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
        int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
        RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);

        // 服务器端设置IsAlive（会自动同步到客户端）
        AFPS_demoCharacter* NewCharacter = Cast<AFPS_demoCharacter>(ElimmedController->GetPawn());
        if (NewCharacter)
        {
            NewCharacter->IsAlive = true;
            NewCharacter->SetMeshVisibility();
            NewCharacter->UpdateHUDHealth();
            NewCharacter->InitInput();
        }
    }
}

void AFPS_demoGameMode::AddReadyTravelPlayer(AFPS_demoPlayerController* PC)
{
    if (!PC || MatchState != MatchState::LeavingMap) return;
    ReadyTravelCount++;

    if (ReadyTravelCount >= GetNumPlayers())
    {
        UKismetSystemLibrary::QuitGame(GetWorld(), GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, false);
    }
}
