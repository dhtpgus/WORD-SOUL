// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterType.h"
#include "NetWork/ClientSocket.h"
#include "Interfaces/HitInterface.h"
#include "WORDSOULCharacter.generated.h"

#define MAX_ATTACK_COMBO 2

class USpringArmComponent;
class UCameraComponent;
class AItem;
class AWeapon;
class UAnimMontage;
class UWORDSOULOverlay;
class UAttributeComponent;


UCLASS()
class WORDSOUL_API AWORDSOULCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	AWORDSOULCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FORCEINLINE void SetOverlappingItem(AItem* Item) { OverlappingItem = Item; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }
	FORCEINLINE void SetCharacterState(ECharacterState state) { CharacterState = state; }
	bool GetIsFalling() const;
	float GetGroundSpeed() const;

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

	void PlayAttackMontage();
	void PlayDodgeMontage();
	void PlayPickupMontage();

	UPROPERTY(VisibleAnywhere)
	UAttributeComponent* Attributes;

	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;
protected:
	virtual void BeginPlay() override;

	void MoveForward(float value);
	void Turn(float value);
	void LookUp(float value);
	void MoveRight(float value);
	void Attack();
	void Dodge();
	void GetItem();
	bool HasEnoughStamina();
	bool IsOccupied();

	UFUNCTION(BlueprintCallable)
	void AttackEnd();

	UFUNCTION(BlueprintCallable)
	void AttackInputChecking();

	bool CanAttack();

	UFUNCTION(BlueprintCallable)
	void DodgeEnd();

	UFUNCTION(BlueprintCallable)
	void PickupEnd();

private:
	void InitWORDSOULOverlay();

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

	UPROPERTY(VisibleAnywhere, Category = Weapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* DodgeMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* PickupMontage;

	UPROPERTY()
	UWORDSOULOverlay* WORDSOULOverlay;

	int32 AttackComboCnt;

	bool bIsAttackButtonClickedWhileAttack;

};
