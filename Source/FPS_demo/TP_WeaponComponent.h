// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "TP_WeaponComponent.generated.h"

class AFPS_demoCharacter;

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPS_DEMO_API UTP_WeaponComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AFPS_demoProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* FireSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* FireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector MuzzleOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* FireMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* FireAction;

	UTP_WeaponComponent();

	UFUNCTION(BlueprintCallable, Category="Weapon")
	bool AttachWeapon(AFPS_demoCharacter* TargetCharacter);

	UPROPERTY(EditAnywhere, Replicated)
	float Damage;

	UFUNCTION(BlueprintCallable)
	void Fire();

	UFUNCTION(BlueprintCallable)
	void StopFire();

	FHitResult HitResult;

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& HitTarget);

	UFUNCTION(NetMulticast, Reliable)	
	void MulticastFire();

	bool IsFiring = false;

	void TraceUnderCrosshair(FHitResult& TraceHitResult);

	FTimerHandle FireTimer;

	UPROPERTY(EditAnywhere)
	float FireDelay = 0.15f;

	void StartFireTimer();
	void FireTimerFinished();

	bool bCanFire = true;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY()
	AFPS_demoCharacter* Character;
};
