// Fill out your copyright notice in the Description page of Project Settings.


#include "BossCharacter.h"
#include "FPS_demoAbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <Net/UnrealNetwork.h>
#include "BossAnimInstance.h"
#include "BossTargetPerceptionComponent.h"
#include <Kismet/GameplayStatics.h>
#include "FPS_demoCharacter.h"

ABossCharacter::ABossCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	NetUpdateFrequency = 20.0f;
	MinNetUpdateFrequency = 5.0f;

	AbilitySystemComponent = CreateDefaultSubobject<UFPS_demoAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	OriginalMaxWalkSpeed = GetCharacterMovement()->GetMaxSpeed();

	TargetPerception = CreateDefaultSubobject<UBossTargetPerceptionComponent>(TEXT("TargetPerception"));
}

UAbilitySystemComponent* ABossCharacter::GetAbilitySystemComponent() const
{
	if (!AbilitySystemComponent) return nullptr;

	return AbilitySystemComponent;
}

void ABossCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	/*if (!IsValid(GetAbilitySystemComponent())) return;

	GetAbilitySystemComponent()->InitAbilityActorInfo(this, this);
	GiveStartupAbilities();*/
}

void ABossCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABossCharacter, Health);
}

void ABossCharacter::OnRep_Health()
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, FString::Printf(TEXT("Health: %.1f"), Health));
	
	if (Health <= 0.0f)
	{
		PlayAnimMontage(DeathAnimMontage);
	}
}

void ABossCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (!IsValid(GetAbilitySystemComponent())) return;

	GetAbilitySystemComponent()->InitAbilityActorInfo(this, this);

	if (!HasAuthority()) return;
	GiveStartupAbilities();
}

void ABossCharacter::GiveStartupAbilities()
{
	for (const auto& Ability : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Ability);
		GetAbilitySystemComponent()->GiveAbility(AbilitySpec);
	}
}

void ABossCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && TargetPerception)
	{
		TargetPlayer = TargetPerception->GetCurrentTarget();
	}
}

void ABossCharacter::AttackForwardArea(float Damage, float AttackRange, float AttackAngleDeg)
{
	if (!HasAuthority())
	{
		return;
	}

	const FVector BossLocation = GetActorLocation();
	const FVector Forward = GetActorForwardVector();
	const float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(AttackAngleDeg * 0.5f));
	const float RangeSq = AttackRange * AttackRange;

	TArray<AActor*> Players;
	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(),
		AFPS_demoCharacter::StaticClass(),
		Players
	);

	for (AActor* Actor : Players)
	{
		AFPS_demoCharacter* Player = Cast<AFPS_demoCharacter>(Actor);
		if (!Player) continue;
		if (Player->Health <= 0.f) continue;

		const FVector ToTarget = Player->GetActorLocation() - BossLocation;
		const float DistSq = ToTarget.SizeSquared();
		if (DistSq > RangeSq) continue;

		const FVector Dir = ToTarget.GetSafeNormal();
		const float Dot = FVector::DotProduct(Forward, Dir);

		// 是否在扇形内
		if (Dot < CosHalfAngle) continue;

		// 命中玩家造成伤害
		UGameplayStatics::ApplyDamage(
			Player,
			Damage,
			GetController(),
			this,
			nullptr
		);
	}

	DrawDebugCone(
		GetWorld(),
		BossLocation,
		Forward,
		AttackRange,
		FMath::DegreesToRadians(AttackAngleDeg * 0.5f),
		FMath::DegreesToRadians(AttackAngleDeg * 0.5f),
		12,
		FColor::Red,
		false,
		1.0f
	);
}
