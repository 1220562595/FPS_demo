// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPS_demoCharacter.h"
#include "FPS_demoProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "TP_WeaponComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "FPS_demo.h"
#include <Net/UnrealNetwork.h>
#include "FPS_demoPlayerController.h"
#include "FPS_demoGameMode.h"
#include "FPS_demoPlayerState.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AFPS_demoCharacter

AFPS_demoCharacter::AFPS_demoCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, BaseEyeHeight));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	//Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	//Mesh1P->SetOnlyOwnerSee(false);
	//Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	//Mesh1P->bCastDynamicShadow = false;
	//Mesh1P->CastShadow = false;
	////Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	//Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	// 创建并设置第一人称网格体（手臂）
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->SetOnlyOwnerSee(true);          // 只有本地玩家能看到
	Mesh1P->bCastDynamicShadow = false;     // 第一人称手臂不投射阴影
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));
	Mesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh1P->SetCollisionResponseToAllChannels(ECR_Ignore);

	// 创建并设置第三人称网格体（全身）
	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh3P"));
	Mesh3P->SetupAttachment(GetRootComponent()); // 根据实际情况附加
	Mesh3P->SetOnlyOwnerSee(false);         // 不是只有拥有者能看到
	Mesh3P->SetOwnerNoSee(true);            // 本地玩家看不到（第一人称时）
	Mesh3P->bCastDynamicShadow = true;
	Mesh3P->CastShadow = true;
	Mesh3P->bCastHiddenShadow = true; // 即使隐藏也投射阴影

	// 创建第一人称武器组件
	FP_WeaponComponent = CreateDefaultSubobject<UTP_WeaponComponent>(TEXT("FP_Weapon"));
	FP_WeaponComponent->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_WeaponComponent->SetOnlyOwnerSee(true);  // 只有拥有者可见

	// 创建第三人称武器组件
	TP_WeaponComponent = CreateDefaultSubobject<UTP_WeaponComponent>(TEXT("TP_Weapon"));
	TP_WeaponComponent->SetupAttachment(Mesh3P, TEXT("GripPoint"));  // 第三人称插槽
	TP_WeaponComponent->SetOwnerNoSee(true);  // 拥有者看不到

	// 在构造函数中设置网络复制
	Mesh1P->SetIsReplicated(false);  // 第一人称网格体不需要复制
	Mesh3P->SetIsReplicated(true);   // 第三人称网格体需要复制

	// 设置碰撞响应
	Mesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh3P->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// 启用网络复制
	bReplicates = true;
	SetReplicateMovement(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	DefaultMeshRelativeLocation = Mesh3P->GetRelativeLocation();

	Turning = ETurning::ET_NotTurning;

	Mesh3P->SetCollisionObjectType(ECC_SkeletalMesh);

	Tags.Add(Player);
}

void AFPS_demoCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPS_demoCharacter, Health);
	DOREPLIFETIME(AFPS_demoCharacter, MaxHealth);
}

void AFPS_demoCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 设置可见性
	SetMeshVisibility();

	UpdateHUDHealth();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AFPS_demoCharacter::ReceiveDamage);
	}
}

void AFPS_demoCharacter::SetMeshVisibility()
{
	if (IsLocallyControlled())
	{
		// 本地玩家：显示第一人称手臂，隐藏第三人称身体
		Mesh1P->SetVisibility(true);
		Mesh1P->SetHiddenInGame(false);

		Mesh3P->SetVisibility(false);
		Mesh3P->SetHiddenInGame(true);
		Mesh3P->SetOwnerNoSee(true);    // 本地玩家看不到自己的全身
	}
	else
	{
		// 远程玩家：显示第三人称身体，隐藏第一人称手臂
		Mesh1P->SetVisibility(false);
		Mesh1P->SetHiddenInGame(true);

		Mesh3P->SetVisibility(true);
		Mesh3P->SetHiddenInGame(false);
		Mesh3P->SetOwnerNoSee(false);   // 其他玩家能看到这个全身
	}
}

void AFPS_demoCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);
}

//////////////////////////////////////////////////////////////////////////// Input

void AFPS_demoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFPS_demoCharacter::Move);

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFPS_demoCharacter::Look);
		
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AFPS_demoCharacter::OnCrouchPressed);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AFPS_demoCharacter::OnCrouchReleased);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AFPS_demoCharacter::PlayFireMontage()
{
	UAnimInstance* AnimInstance = Mesh3P->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		AnimInstance->Montage_JumpToSection(FName("Fire"));
	}
}

void AFPS_demoCharacter::PlayHitReactMontage()
{
	UAnimInstance* AnimInstance = Mesh3P->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
	}
}

void AFPS_demoCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = Mesh3P->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
		FName SectionName("fromfront");
		AnimInstance->Montage_JumpToSection(FName("SectionName"));
	}
}

void AFPS_demoCharacter::OnRep_Health()
{
	UpdateHUDHealth();
}

void AFPS_demoCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);
	UpdateHUDHealth();

	if (Health == 0.0f)
	{
		AFPS_demoGameMode* FPS_demoGameMode = GetWorld()->GetAuthGameMode<AFPS_demoGameMode>();
		if (FPS_demoGameMode)
		{
			PlayerController = PlayerController == nullptr ? Cast<AFPS_demoPlayerController>(Controller) : PlayerController;
			if (AFPS_demoPlayerController* AttackerController = Cast<AFPS_demoPlayerController>(InstigatorController))
			{
				if (AFPS_demoPlayerState* AttackerPlayerState = AttackerController->GetPlayerState<AFPS_demoPlayerState>())
				{
					AttackerPlayerState->AddKillPlayer();
				}
				Mesh1P->SetVisibility(false);
				FP_WeaponComponent->SetVisibility(false);
				FPS_demoGameMode->PlayerEliminated(this, PlayerController, AttackerController);
			}
		}
	}
}

void AFPS_demoCharacter::UpdateHUDHealth()
{
	PlayerController = PlayerController == nullptr ? Cast<AFPS_demoPlayerController>(Controller) : PlayerController;
	if (PlayerController)
	{
		/*GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
			TEXT("111111111111"));*/
		PlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void AFPS_demoCharacter::Elim()
{
	IsAlive = false;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MulticastElim();
	GetWorldTimerManager().SetTimer(ElimTimer, this, &AFPS_demoCharacter::ElimTimerFinished, ElimDelay);
}

void AFPS_demoCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
}

void AFPS_demoCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	// 当客户端的Controller变化（重生时会触发），且角色存活时初始化输入
	if (IsAlive && GetController())
	{
		Client_InitInput();
	}
}

void AFPS_demoCharacter::Client_InitInput_Implementation()
{
	InitInput();
}

void AFPS_demoCharacter::InitInput()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !PC->GetLocalPlayer()) return;

	// 1. 重新添加输入映射上下文（IMC）
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (InputSubsystem)
	{
		// 先移除再添加，确保路由不中断
		if (AFPS_demoPlayerController* FPSPC = Cast<AFPS_demoPlayerController>(PC))
		{
			if (FPSPC->InputMappingContext)
			{
				InputSubsystem->RemoveMappingContext(FPSPC->InputMappingContext);
				InputSubsystem->AddMappingContext(FPSPC->InputMappingContext, 0);
			}
		}

		// 重新添加武器的IMC
		if (GetFP_WeaponComponent() && GetFP_WeaponComponent()->FireMappingContext)
		{
			InputSubsystem->RemoveMappingContext(GetFP_WeaponComponent()->FireMappingContext);
			InputSubsystem->AddMappingContext(GetFP_WeaponComponent()->FireMappingContext, 1);
		}
		if (GetTP_WeaponComponent() && GetTP_WeaponComponent()->FireMappingContext)
		{
			InputSubsystem->RemoveMappingContext(GetTP_WeaponComponent()->FireMappingContext);
			InputSubsystem->AddMappingContext(GetTP_WeaponComponent()->FireMappingContext, 1);
		}
	}

	// 2. 重新绑定输入Action（核心：添加移动输入绑定）
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PC->InputComponent))
	{
		// 绑定移动输入（之前缺失的关键步骤）
		if (AFPS_demoPlayerController* FPSPC = Cast<AFPS_demoPlayerController>(PC))
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFPS_demoCharacter::Move);

			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFPS_demoCharacter::Look);

			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AFPS_demoCharacter::OnCrouchPressed);
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AFPS_demoCharacter::OnCrouchReleased);
		}

		// 绑定武器开火
		if (GetFP_WeaponComponent())
		{
			EnhancedInputComponent->BindAction(GetFP_WeaponComponent()->FireAction, ETriggerEvent::Triggered, GetFP_WeaponComponent(), &UTP_WeaponComponent::Fire);
			EnhancedInputComponent->BindAction(GetFP_WeaponComponent()->FireAction, ETriggerEvent::Completed, GetFP_WeaponComponent(), &UTP_WeaponComponent::StopFire);
		}
		if (GetTP_WeaponComponent())
		{
			EnhancedInputComponent->BindAction(GetTP_WeaponComponent()->FireAction, ETriggerEvent::Triggered, GetTP_WeaponComponent(), &UTP_WeaponComponent::Fire);
			EnhancedInputComponent->BindAction(GetTP_WeaponComponent()->FireAction, ETriggerEvent::Completed, GetTP_WeaponComponent(), &UTP_WeaponComponent::StopFire);
		}
	}
}

void AFPS_demoCharacter::MulticastElim_Implementation()
{
	bElimmed = true;
	PlayElimMontage();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (PlayerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* SubSys =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (FP_WeaponComponent && FP_WeaponComponent->FireMappingContext)
				SubSys->RemoveMappingContext(FP_WeaponComponent->FireMappingContext);

			if (TP_WeaponComponent && TP_WeaponComponent->FireMappingContext)
				SubSys->RemoveMappingContext(TP_WeaponComponent->FireMappingContext);

			if (PlayerController->InputMappingContext)
				SubSys->RemoveMappingContext(PlayerController->InputMappingContext);
		}
	}
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh1P->SetVisibility(false);
	FP_WeaponComponent->SetVisibility(false);
}

void AFPS_demoCharacter::ElimTimerFinished()
{
	AFPS_demoGameMode* FPS_demoGameMode = GetWorld()->GetAuthGameMode<AFPS_demoGameMode>();
	if (FPS_demoGameMode)
	{
		FPS_demoGameMode->RequestRespawn(this, Controller);
	}
}

void AFPS_demoCharacter::OnElimMontageFinished()
{
	/*GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
		TEXT("1"));*/
	Mesh3P->SetVisibility(false);
	Mesh3P->SetCastShadow(false);
	TP_WeaponComponent->SetVisibility(false); 
	TP_WeaponComponent->SetCastShadow(false);
}

void AFPS_demoCharacter::Move(const FInputActionValue& Value)
{
	IsWalking = true;

	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AFPS_demoCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AFPS_demoCharacter::OnCrouchPressed(const FInputActionValue& Value)
{
	if (!bIsCrouched && !GetCharacterMovement()->IsFalling())           
		Crouch();
}

void AFPS_demoCharacter::OnCrouchReleased(const FInputActionValue& Value)
{
	if (bIsCrouched)
		UnCrouch();
}

void AFPS_demoCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	const float StandHalfHeight = GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float CrouchHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float DeltaZ = StandHalfHeight - CrouchHalfHeight; // 正值

	Mesh3P->SetRelativeLocation(DefaultMeshRelativeLocation + FVector(0, 0, -DeltaZ - 13));

	if (TP_WeaponComponent)
	{
		TP_WeaponComponent->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		TP_WeaponComponent->AttachToComponent(Mesh3P, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("GripPoint"));
	}
}

void AFPS_demoCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	// 完全恢复出厂设置
	const FVector DefaultMeshLoc = GetClass()->GetDefaultObject<AFPS_demoCharacter>()->Mesh3P->GetRelativeLocation();
	Mesh3P->SetRelativeLocation(DefaultMeshLoc);

	if (TP_WeaponComponent)
	{
		TP_WeaponComponent->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		TP_WeaponComponent->AttachToComponent(Mesh3P, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("GripPoint"));
	}
}

void AFPS_demoCharacter::AimOffset(float DeltaTime)
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.0f && !bIsInAir)
	{
		FRotator CurrentAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (Turning == ETurning::ET_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.0f || bIsInAir)
	{
		StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		AO_Yaw = 0.0f;
		bUseControllerRotationYaw = true;
		Turning = ETurning::ET_NotTurning;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.0f && !IsLocallyControlled())
	{
		FVector2D InRange(270.0f, 360.0f);
		FVector2D OutRange(-90.0f, 0.0f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void AFPS_demoCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.0f)
	{
		Turning = ETurning::ET_Right;
	}
	else if (AO_Yaw < -90.0f)
	{
		Turning = ETurning::ET_Left;
	}
	if (Turning != ETurning::ET_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.0f, DeltaTime, 5.0f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.0f)
		{
			Turning = ETurning::ET_NotTurning;
			StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		}
	}
}

//void AFPS_demoCharacter::OnRep_PlayerState()
//{
//	Super::OnRep_PlayerState();
//
//	AFPS_demoPlayerState* PS =
//		GetPlayerState<AFPS_demoPlayerState>();
//
//	if (!PS) return;
//
//	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
//		TEXT("111111111111"));
//
//	/*GetCharacterMovement()->MaxWalkSpeed += PS->BonusMoveSpeed;
//	UpdateHUDHealth();*/
//	ApplyPlayerStateBonuses();
//}

void AFPS_demoCharacter::ApplyPlayerStateBonuses()
{
	AFPS_demoPlayerState* PS =
		GetPlayerState<AFPS_demoPlayerState>();

	if (!PS) return;

	if (IsLocallyControlled())
	{
		MaxHealth += PS->BonusMaxHealth;
		GetCharacterMovement()->MaxWalkSpeed += PS->BonusMoveSpeed;
		GetFP_WeaponComponent()->Damage += PS->BonusDamage;

		Health = MaxHealth;
		UpdateHUDHealth();
	}
	else
	{
		ClientApplyPlayerStateBonuses();
	}
}

void AFPS_demoCharacter::ClientApplyPlayerStateBonuses_Implementation()
{
	AFPS_demoPlayerState* PS =
		GetPlayerState<AFPS_demoPlayerState>();

	if (!PS) return;

	MaxHealth += PS->BonusMaxHealth;
	GetCharacterMovement()->MaxWalkSpeed += PS->BonusMoveSpeed;
	GetFP_WeaponComponent()->Damage += PS->BonusDamage;

	Health = MaxHealth;
	UpdateHUDHealth();
	UpdateHUDHealth();
}
