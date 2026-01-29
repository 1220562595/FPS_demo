// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPS_demoProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "FPS_demoCharacter.h"
#include "TP_WeaponComponent.h"
#include "FPS_demo.h"
#include "BossCharacter.h"
#include "FPS_demoAbilitySystemComponent.h"
#include "Tags.h"
#include "FPS_demoPlayerState.h"

AFPS_demoProjectile::AFPS_demoProjectile() 
{
	bReplicates = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);
	CollisionComp->OnComponentHit.AddDynamic(this, &AFPS_demoProjectile::OnHit);
	

	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	RootComponent = CollisionComp;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	InitialLifeSpan = 3.0f;
}

void AFPS_demoProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr) && (SourceWeapon != nullptr)/* && OtherComp->IsSimulatingPhysics()*/)
	{
		//OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());

		AFPS_demoCharacter* HurtCharacter = Cast<AFPS_demoCharacter>(OtherActor);
		if (HurtCharacter)
		{
			HurtCharacter->PlayHitReactMontage();
		}

		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
		}
		if (ImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
		}

		if (HasAuthority())
		{
			AFPS_demoCharacter* OwnerCharacter = Cast<AFPS_demoCharacter>(GetOwner());
			AController* OwnerController = nullptr;
			if (OwnerCharacter)
			{
				OwnerController = OwnerCharacter->Controller;
			}

			if (!OwnerController) return;

			float DamageToCause = Hit.BoneName.ToString() == FString("head") ? (SourceWeapon->Damage * 2) : SourceWeapon->Damage;
			if (HurtCharacter)
			{
				if (AFPS_demoPlayerState* OwnerPlayerState = OwnerController->GetPlayerState<AFPS_demoPlayerState>())
				{
					OwnerPlayerState->AddDamageToPlayer(DamageToCause);
				}
				UGameplayStatics::ApplyDamage(OtherActor, DamageToCause, OwnerController, this, UDamageType::StaticClass());
			}
			else if (ABossCharacter* HurtBoss = Cast<ABossCharacter>(OtherActor))
			{
				if (AFPS_demoPlayerState* OwnerPlayerState = OwnerController->GetPlayerState<AFPS_demoPlayerState>())
				{
					OwnerPlayerState->AddDamageToBoss(DamageToCause);
				}
				UAbilitySystemComponent* BossASC = HurtBoss->GetAbilitySystemComponent();
				if (IsValid(BossASC))
				{
					FGameplayEventData EventData;
					EventData.Instigator = GetOwner(); // 子弹所有者（玩家）
					EventData.Target = HurtBoss;       // 被击中的Boss
					EventData.EventTag = Tags::Events::HitReact; // 事件标签
					EventData.ContextHandle = FGameplayEffectContextHandle();
					EventData.EventMagnitude = DamageToCause;

					// 发送GAS事件：触发Boss的回调
					BossASC->HandleGameplayEvent(Tags::Events::HitReact, &EventData);
				}
			}
		}

		Destroy();
	}
}

void AFPS_demoProjectile::BeginPlay()
{
	Super::BeginPlay();

	AFPS_demoCharacter* Character = Cast<AFPS_demoCharacter>(GetOwner());
	if (Character)
	{
		APawn* OwnerPawn = Cast<APawn>(Character);
		if (OwnerPawn)
		{
			CollisionComp->IgnoreActorWhenMoving(OwnerPawn, true);
			OwnerPawn->MoveIgnoreActorAdd(this);
		}
	}
}

//void AFPS_demoProjectile::Destroyed()
//{
//	Super::Destroyed();
//
//	if (ImpactParticles)
//	{
//		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
//	}
//	if (ImpactSound)
//	{
//		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
//	}
//}
