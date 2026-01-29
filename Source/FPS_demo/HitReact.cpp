// Fill out your copyright notice in the Description page of Project Settings.


#include "HitReact.h"
#include "BossCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "FPS_demoCharacter.h"
#include "TP_WeaponComponent.h"
#include "BossAIController.h"
#include "Components/CapsuleComponent.h"
#include "FPS_demoGameMode.h"
#include "FPS_demoPlayerState.h"

void UHitReact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// 1. 基础校验：仅在服务器端执行、ActorInfo和Avatar有效
	if (!HasAuthority(&ActivationInfo) || !ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		UE_LOG(LogTemp, Warning, TEXT("UHitReact: 权限或Actor无效，终止能力"));
		return;
	}

	// 2. 校验Gameplay Tag是否配置（防止忘记在编辑器设置）
	if (!HitReactEventTag.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		UE_LOG(LogTemp, Error, TEXT("UHitReact: HitReactEventTag未配置！"));
		return;
	}

	// 3. 创建等待Gameplay Event的任务
	WaitHitReactEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,               // 所属Ability
		HitReactEventTag,   // 要监听的Gameplay Tag
		nullptr             // 可选：仅监听指定Instigator的事件（nullptr表示监听所有）
	);

	// 4. 绑定回调：事件触发时执行OnWaitEventCompleted
	if (WaitHitReactEventTask)
	{
		// 绑定动态多播委托到自定义回调函数
		WaitHitReactEventTask->EventReceived.AddDynamic(this, &UHitReact::OnWaitEventCompleted);

		// 5. 激活任务：开始监听事件
		WaitHitReactEventTask->Activate();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		UE_LOG(LogTemp, Error, TEXT("UHitReact: 创建WaitHitReactEventTask失败！"));
	}
}

void UHitReact::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Health"));
	if (!HasAuthority(&ActivationInfo) || !ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return;
	}

	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return;
	}

	ABossCharacter* OwnerCharacter = Cast<ABossCharacter>(ActorInfo->AvatarActor.Get());

	if (!OwnerCharacter) return;

	UCharacterMovementComponent* CharacterMovementComponent = OwnerCharacter->GetCharacterMovement();
	if (!CharacterMovementComponent)
	{
		return;
	}

	if (OwnerCharacter->bHasAppliedSlow) // BossCharacter恢复
	{
		CharacterMovementComponent->MaxWalkSpeed = OwnerCharacter->OriginalMaxWalkSpeed;
		OwnerCharacter->bHasAppliedSlow = false;
		OwnerCharacter->OriginalMaxWalkSpeed = 0.0f;
	}


	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHitReact::OnWaitEventCompleted(FGameplayEventData Payload)
{
	FGameplayAbilitySpecHandle Handle = GetCurrentAbilitySpecHandle();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	const FGameplayAbilityActivationInfo ActivationInfo = GetCurrentActivationInfo();
	
	if (!HasAuthority(&ActivationInfo) || !ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	ABossCharacter* OwnerCharacter = Cast<ABossCharacter>(ActorInfo->AvatarActor.Get());
	if (!OwnerCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UCharacterMovementComponent* CharacterMovementComponent = OwnerCharacter->GetCharacterMovement();
	if (!CharacterMovementComponent)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (!OwnerCharacter->bHasAppliedSlow)
	{
		OwnerCharacter->OriginalMaxWalkSpeed = CharacterMovementComponent->MaxWalkSpeed;
		OwnerCharacter->bHasAppliedSlow = true;
	}

	float SlowSpeed = OwnerCharacter->OriginalMaxWalkSpeed * SlowDownRatio;
	CharacterMovementComponent->MaxWalkSpeed = SlowSpeed;

	FTimerHandle RestoreTimer;
	GetWorld()->GetTimerManager().SetTimer(RestoreTimer, [=]()
		{
			if (OwnerCharacter->bHasAppliedSlow == false) return;
			OwnerCharacter->bHasAppliedSlow = false;
			if (CharacterMovementComponent)
			{
				CharacterMovementComponent->MaxWalkSpeed = OwnerCharacter->OriginalMaxWalkSpeed;
			}
		}, 0.5f, false);

	const AFPS_demoCharacter* Attacker = Cast<AFPS_demoCharacter>(Payload.Instigator);
	if (!Attacker)
	{
		return;
	}

	float AttackerDamage = Payload.EventMagnitude;
	OwnerCharacter->Health -= AttackerDamage;
	// 防止血量为负数
	OwnerCharacter->Health = FMath::Max(0.0f, OwnerCharacter->Health);
	if (OwnerCharacter->Health <= 0.0f)
	{
		AController* Controller = Attacker->GetController();
		if (Controller)
		{
			APlayerState* PS = Controller->PlayerState;
			if (PS)
			{
				if (AFPS_demoPlayerState* AttackerPlayerState = Cast<AFPS_demoPlayerState>(PS))
				{
					AttackerPlayerState->AddKillBoss();
				}
				if (PS->GetClass()->ImplementsInterface(URewardReceiverInterface::StaticClass()))
				{
					IRewardReceiverInterface::Execute_ReceiveKillReward(PS);
				}
			}
		}

		// 停止AI控制器
		ABossAIController* BossAIController = Cast<ABossAIController>(OwnerCharacter->GetController());
		if (BossAIController)
		{
			BossAIController->StopMovement(); // 停止移动
			BossAIController->UnPossess();    // 解除对Boss的控制
		}

		// 3. 禁用碰撞和移动组件
		OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		OwnerCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CharacterMovementComponent->StopMovementImmediately(); // 立即停止移动
		CharacterMovementComponent->SetMovementMode(MOVE_None); // 禁用移动模式

		// 4. 清理减速状态
		if (OwnerCharacter->bHasAppliedSlow)
		{
			CharacterMovementComponent->MaxWalkSpeed = OwnerCharacter->OriginalMaxWalkSpeed;
			OwnerCharacter->bHasAppliedSlow = false;
			OwnerCharacter->OriginalMaxWalkSpeed = 0.0f;
		}

		OwnerCharacter->PlayAnimMontage(OwnerCharacter->DeathAnimMontage);
	}
}
