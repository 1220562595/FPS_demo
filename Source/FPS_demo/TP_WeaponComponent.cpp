// Copyright Epic Games, Inc. All Rights Reserved.


#include "TP_WeaponComponent.h"
#include "FPS_demoCharacter.h"
#include "FPS_demoProjectile.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Animation/AnimInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Engine/SkeletalMeshSocket.h"
#include "TimerManager.h"

void UTP_WeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTP_WeaponComponent, Damage);
}

UTP_WeaponComponent::UTP_WeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTP_WeaponComponent::TraceUnderCrosshair(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;
		FVector End = Start + CrosshairWorldDirection * 80000.0f;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Character);
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility,
			Params
		);
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
		else
		{
			End = TraceHitResult.ImpactPoint;
		}
	}
}

void UTP_WeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("weapon beginplay"));
	Character = Cast<AFPS_demoCharacter>(GetOwner());
	if (!Character) return;
	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(FireMappingContext, 1);
		}

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &UTP_WeaponComponent::Fire);
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &UTP_WeaponComponent::StopFire);
		}
	}
}

void UTP_WeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UTP_WeaponComponent::Fire()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("isfiring:%d"), IsFiring));
	if (Character == nullptr || Character->GetController() == nullptr)
	{
		return;
	}

	if (!bCanFire) return;
	IsFiring = true;

	TraceUnderCrosshair(HitResult);
	ServerFire(HitResult.ImpactPoint);
	bCanFire = false;
	StartFireTimer();
}

void UTP_WeaponComponent::StopFire()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("stop"));
	IsFiring = false;
}

void UTP_WeaponComponent::StartFireTimer()
{
	if (!Character) return;
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("isfiring:%d"), IsFiring));
	Character->GetWorldTimerManager().SetTimer(
		FireTimer, 
		this, 
		&UTP_WeaponComponent::FireTimerFinished, 
		FireDelay
	);
}

void UTP_WeaponComponent::FireTimerFinished()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("isfiring:%d"),IsFiring));
	bCanFire = true;
	if (IsFiring)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("finished"));
		TraceUnderCrosshair(HitResult);
		ServerFire(HitResult.ImpactPoint);
		bCanFire = false;
		StartFireTimer();
	}
}

void UTP_WeaponComponent::ServerFire_Implementation(const FVector_NetQuantize& HitTarget)
{
	if (ProjectileClass != nullptr)
	{
		//UWorld* const World = GetWorld();
		//if (World != nullptr)
		//{
		//	IsFiring = true;
		//	APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
		//	const FRotator SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
		//	const FVector SpawnLocation = GetSocketLocation(FName("Muzzle")) /*GetOwner()->GetActorLocation()*/ +SpawnRotation.RotateVector(MuzzleOffset);

		//	FActorSpawnParameters ActorSpawnParams;
		//	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		//	World->SpawnActor<AFPS_demoProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		//}
		const USkeletalMeshSocket* MuzzleSocket = GetSocketByName(FName("Muzzle"));
		if (MuzzleSocket)
		{
			FTransform SocketTransform = MuzzleSocket->GetSocketTransform(this);
			FVector SocketLocation = SocketTransform.GetLocation();
			UWorld* World = GetWorld();
			if (World)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = Character;
				APawn* Instigator = Cast<APawn>(Character);
				if (Instigator)
				{
					SpawnParams.Instigator = Instigator;
				}
				AFPS_demoProjectile* Projectile = World->SpawnActor<AFPS_demoProjectile>(
					ProjectileClass,
					SocketLocation + MuzzleOffset,
					(HitTarget - SocketLocation).Rotation(),
					SpawnParams
				);
				if (Projectile)
				{
					Projectile->SourceWeapon = this;
				}
			}
		}
	}
	MulticastFire();
}

void UTP_WeaponComponent::MulticastFire_Implementation()
{
	if (!Character || Character->IsPendingKillPending()) return;

	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
	}

	if (FireAnimation != nullptr)
	{
		UAnimInstance* AnimInstance = Character->GetMesh1P()->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}

	Character->PlayFireMontage();
}

bool UTP_WeaponComponent::AttachWeapon(AFPS_demoCharacter* TargetCharacter)
{
	/*Character = TargetCharacter;

	if (Character == nullptr || Character->GetInstanceComponents().FindItemByClass<UTP_WeaponComponent>())
	{
		return false;
	}

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	AttachToComponent(Character->GetMesh1P(), AttachmentRules, FName(TEXT("GripPoint")));

	Character->AddInstanceComponent(this);

	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(FireMappingContext, 1);
		}

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &UTP_WeaponComponent::Fire);
		}
	}
	*/
	return true;
}

void UTP_WeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
//	if (Character == nullptr)
//	{
//		return;
//	}
//
//	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
//	{
//		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
//		{
//			Subsystem->RemoveMappingContext(FireMappingContext);
//		}
//	}
}