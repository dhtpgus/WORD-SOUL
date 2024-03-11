// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/WORDSOULCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/AttributeComponent.h"
#include "Items/Item.h"
#include "Items/Weapons/Weapon.h"
#include "Animation/AnimMontage.h"
#include "HUD/WORDSOULHUD.h"
#include "HUD/WORDSOULOverlay.h"

// Sets default values
AWORDSOULCharacter::AWORDSOULCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));
}

// Called when the game starts or when spawned
void AWORDSOULCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	InitWORDSOULOverlay();
}

void AWORDSOULCharacter::InitWORDSOULOverlay()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		AWORDSOULHUD* WORDSOULHUD = Cast<AWORDSOULHUD>(PlayerController->GetHUD());
		if (WORDSOULHUD)
		{
			WORDSOULOverlay = WORDSOULHUD->GetWORDSOULOverlay();
			if (WORDSOULOverlay)
			{
				WORDSOULOverlay->SetHealthBarPercent(1.f);
				WORDSOULOverlay->SetStaminaBarPercent(1.f);
			}
		}
	}
}

// Called every frame
void AWORDSOULCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Attributes and WORDSOULOverlay)
	{
		Attributes->RegenStamina(DeltaTime);
		WORDSOULOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}
}

// Called to bind functionality to input
void AWORDSOULCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &AWORDSOULCharacter::MoveForward);
	PlayerInputComponent->BindAxis(FName("Turn"), this, &AWORDSOULCharacter::Turn);
	PlayerInputComponent->BindAxis(FName("LookUp"), this, &AWORDSOULCharacter::LookUp);
	PlayerInputComponent->BindAxis(FName("MoveRight"), this, &AWORDSOULCharacter::MoveRight);

	PlayerInputComponent->BindAction(FName("Jump"), IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(FName("GetItem"), IE_Pressed, this, &AWORDSOULCharacter::GetItem);
	PlayerInputComponent->BindAction(FName("Attack"), IE_Pressed, this, &AWORDSOULCharacter::Attack);
	PlayerInputComponent->BindAction(FName("Dodge"), IE_Pressed, this, &AWORDSOULCharacter::Dodge);
}

void AWORDSOULCharacter::MoveForward(float value)
{
	if (ActionState == EActionState::EAS_Attacking) return;
	if (Controller && value != 0.f)
	{
		const FRotator ControlRotation = GetControlRotation();
		const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, value);
	}
}

void AWORDSOULCharacter::Turn(float value)
{
	AddControllerYawInput(value);
}

void AWORDSOULCharacter::LookUp(float value)
{
	AddControllerPitchInput(value);
}

void AWORDSOULCharacter::MoveRight(float value)
{
	if (ActionState == EActionState::EAS_Attacking) return;
	if (Controller && value != 0.f)
	{
		const FRotator ControlRotation = GetControlRotation();
		const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, value);
	}
}

void AWORDSOULCharacter::GetItem()
{
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	if (OverlappingItem)
	{
		if (OverlappingWeapon)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(1, 30.f, FColor::Red, "GetItem");
			}

			OverlappingWeapon->Equip(GetMesh(), FName("RightHandSocket"));
			CharacterState = ECharacterState::ECS_EquippedWeapon;
		}
	}

	
}

bool AWORDSOULCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied and CharacterState != ECharacterState::ECS_Unequipped;
}

void AWORDSOULCharacter::DodgeEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void AWORDSOULCharacter::Attack()
{
	if (CanAttack())
	{
		PlayAttackMontage();
		ActionState = EActionState::EAS_Attacking;
	}
	
}

void AWORDSOULCharacter::Dodge()
{
	if (IsOccupied() or !HasEnoughStamina()) return;

	PlayDodgeMontage();
	ActionState = EActionState::EAS_Dodge;
	if (Attributes and WORDSOULOverlay)
	{
		Attributes->UseStamina(Attributes->GetDodgeCost());
		WORDSOULOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}
}

bool AWORDSOULCharacter::HasEnoughStamina()
{
	return Attributes and Attributes->GetStamina() > Attributes->GetDodgeCost();
}

bool AWORDSOULCharacter::IsOccupied()
{
	return ActionState != EActionState::EAS_Unoccupied;
}

void AWORDSOULCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}



void AWORDSOULCharacter::PlayAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance and AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage);
		const int32 MontageSelection = FMath::RandRange(0, 1);
		FName SectionName = FName();

		switch (MontageSelection)
		{
		case 0:
			SectionName = FName("Attack1");
			break;
		case 1:
			SectionName = FName("Attack2");
			break;
		default:
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
	}
}

void AWORDSOULCharacter::PlayDodgeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance and DodgeMontage)
	{
		AnimInstance->Montage_Play(DodgeMontage);
		AnimInstance->Montage_JumpToSection("Default", DodgeMontage);
	}
}



