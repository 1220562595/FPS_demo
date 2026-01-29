// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUD.h"
#include "CharacterOverlay.h"
#include "MatchEnd.h"

void APlayerHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f);

		if (Crosshair)
		{
			DrawCrosshair(Crosshair, ViewportCenter);
		}
	}
}

void APlayerHUD::BeginPlay()
{
	Super::BeginPlay();

	AddCharacterOverlay();
}

void APlayerHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void APlayerHUD::AddMatchEnd()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && MatchEndClass)
	{
		MatchEnd = CreateWidget<UMatchEnd>(PlayerController, MatchEndClass);
	}
}

void APlayerHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter)
{
	float TextureWidth = Texture->GetSizeX();
	float TextureHeight = Texture->GetSizeY();
	FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.0f),
		ViewportCenter.Y - (TextureHeight / 2.0f)
	);

	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.0f,
		0.0f,
		1.0f,
		1.0f,
		FLinearColor::White
	);
}
