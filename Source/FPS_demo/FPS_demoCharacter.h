// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Types/Turning.h"

#include "FPS_demoCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;
class UTP_WeaponComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UENUM(BlueprintType)
enum class EPlayerRewardType : uint8
{
	IncreaseMaxHealth UMETA(DisplayName = "Increase Max Health"),
	Heal UMETA(DisplayName = "Heal"),
	IncreaseMoveSpeed UMETA(DisplayName = "Increase Move Speed"),
};

UCLASS(config=Game)
class AFPS_demoCharacter : public ACharacter
{
	GENERATED_BODY()

	friend class UPlayerAnimInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh3P;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	bool IsWalking;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	// 第一人称武器组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	UTP_WeaponComponent* FP_WeaponComponent;

	// 第三人称武器组件（其他玩家可见）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	UTP_WeaponComponent* TP_WeaponComponent;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ElimMontage;
	
public:
	AFPS_demoCharacter();
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	void SetMeshVisibility();

	void PlayFireMontage();
	void PlayHitReactMontage();
	void PlayElimMontage();

	UPROPERTY(EditAnywhere, Replicated)
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Health)
	float Health;

	UFUNCTION()
	void OnRep_Health();

	class AFPS_demoPlayerController* PlayerController;

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	void UpdateHUDHealth();

	void Elim();

	bool IsAlive = true;

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;
	void InitInput();

	// 客户端本地函数：初始化输入
	UFUNCTION(Client, Reliable)
	void Client_InitInput();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	bool bElimmed = false;

	FTimerHandle ElimTimer;
	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.0f;

	void ElimTimerFinished();

	UFUNCTION(BlueprintCallable)
	void OnElimMontageFinished();

	//virtual void OnRep_PlayerState() override;
	void ApplyPlayerStateBonuses();

	UFUNCTION(Client, Reliable)
	void ClientApplyPlayerStateBonuses();

protected:
	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void OnCrouchPressed(const FInputActionValue& Value);
	void OnCrouchReleased(const FInputActionValue& Value);

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	UPROPERTY()
	FVector DefaultMeshRelativeLocation;   // 站立时的初始偏移

	void AimOffset(float DeltaTime);

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurning Turning;
	void TurnInPlace(float DeltaTime);

	const FName Player = FName("Player");

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

public:
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE ETurning GetTurning() const { return Turning; }
	FORCEINLINE UTP_WeaponComponent* GetFP_WeaponComponent() { return FP_WeaponComponent; }
	FORCEINLINE UTP_WeaponComponent* GetTP_WeaponComponent() { return TP_WeaponComponent; }
};

