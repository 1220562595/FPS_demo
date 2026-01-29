// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"
#include "FPS_demoCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TP_WeaponComponent.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	PlayerCharacter = Cast<AFPS_demoCharacter>(TryGetPawnOwner());
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (PlayerCharacter == nullptr) PlayerCharacter = Cast<AFPS_demoCharacter>(TryGetPawnOwner());
	if (PlayerCharacter == nullptr) return;
	if (Weapon == nullptr) Weapon = PlayerCharacter->TP_WeaponComponent;
	if (Weapon == nullptr) return;

	FVector WorldVelocity = PlayerCharacter->GetVelocity();
	WorldVelocity.Z = 0.0f;
	Speed = WorldVelocity.Size();
	LocalVelocity = PlayerCharacter->GetActorRotation().UnrotateVector(WorldVelocity);
	Direction = FMath::Atan2(LocalVelocity.Y, LocalVelocity.X) * 180.f / PI;

	bIsInAir = PlayerCharacter->GetCharacterMovement()->IsFalling();
	bIsCrouched = PlayerCharacter->bIsCrouched;

	AO_Yaw = PlayerCharacter->GetAO_Yaw();
	AO_Pitch = PlayerCharacter->GetAO_Pitch();
	Turning = PlayerCharacter->GetTurning();

	bIsFiring = Weapon->IsFiring;

	bElimmed = PlayerCharacter->bElimmed;
}
