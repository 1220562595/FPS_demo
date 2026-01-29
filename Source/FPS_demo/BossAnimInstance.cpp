// Fill out your copyright notice in the Description page of Project Settings.


#include "BossAnimInstance.h"
#include "BossCharacter.h"

void UBossAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BossCharacter = Cast<ABossCharacter>(TryGetPawnOwner());
}

void UBossAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (BossCharacter == nullptr) BossCharacter = Cast<ABossCharacter>(TryGetPawnOwner());
	if (BossCharacter == nullptr) return;

	FVector WorldVelocity = BossCharacter->GetVelocity();
	WorldVelocity.Z = 0.0f;
	Speed = WorldVelocity.Size();
}
