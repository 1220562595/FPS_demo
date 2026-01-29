// Fill out your copyright notice in the Description page of Project Settings.


#include "MatchEnd.h"
#include "FPS_demoPlayerController.h"
#include "Components/Button.h"

void UMatchEnd::NativeConstruct()
{
    Super::NativeConstruct();

    if (ReturnLobby)
    {
        ReturnLobby->OnClicked.AddDynamic(this, &UMatchEnd::OnReturnLobbyClicked);
    }
}

void UMatchEnd::NativeDestruct()
{
    if (ReturnLobby)
    {
        ReturnLobby->OnClicked.RemoveDynamic(this, &UMatchEnd::OnReturnLobbyClicked);
    }
    Super::NativeDestruct();
}

void UMatchEnd::OnReturnLobbyClicked()
{
    if (AFPS_demoPlayerController* PC = Cast<AFPS_demoPlayerController>(GetOwningPlayer()))
    {
        PC->ServerReadyToTravel();
    }
}