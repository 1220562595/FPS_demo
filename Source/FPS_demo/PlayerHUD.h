// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"

/**
 * 
 */
UCLASS()
class FPS_DEMO_API APlayerHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> CharacterOverlayClass;

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	void AddCharacterOverlay();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> MatchEndClass;

	UPROPERTY()
	class UMatchEnd* MatchEnd;

	void AddMatchEnd();

private:
	UPROPERTY(EditAnywhere)
	UTexture2D* Crosshair;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter);
};
