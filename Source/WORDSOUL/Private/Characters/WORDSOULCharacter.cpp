// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/WORDSOULCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

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
}

// Called when the game starts or when spawned
void AWORDSOULCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWORDSOULCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
}

void AWORDSOULCharacter::MoveForward(float value)
{
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
	if (OverlappingItem)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 30.f, FColor::Red, "GetItem");
		}
	}
}

