// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MatchEnd.generated.h"

/**
 * 
 */
UCLASS()
class FPS_DEMO_API UMatchEnd : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Ranking;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	class UButton* ReturnLobby;

	UFUNCTION()
	void OnReturnLobbyClicked();
};
