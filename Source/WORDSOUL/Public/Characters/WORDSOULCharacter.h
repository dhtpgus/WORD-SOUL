// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterType.h"
#include "NetWork/ClientSocket.h"
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
class WORDSOUL_API AWORDSOULCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AWORDSOULCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FORCEINLINE void SetOverlappingItem(AItem* Item) { OverlappingItem = Item; }
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

protected:
	virtual void BeginPlay() override;

	void MoveForward(float value);
	void Turn(float value);
	void LookUp(float value);
	void MoveRight(float value);
	void GetItem();
	void Attack();
	void Dodge();

	bool HasEnoughStamina();
	bool IsOccupied();

	void PlayAttackMontage();
	void PlayDodgeMontage();

	UFUNCTION(BlueprintCallable)
	void AttackEnd();

	UFUNCTION(BlueprintCallable)
	void AttackInputChecking();

	bool CanAttack();

	UFUNCTION(BlueprintCallable)
	void DodgeEnd();

	UPROPERTY(EditAnywhere, Category = "SpawnCharacter")
	TSubclassOf<class ACharacter> SpawnCharacter;

private:
	UPROPERTY(VisibleAnywhere)
	UAttributeComponent* Attributes;

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

	UPROPERTY()
	UWORDSOULOverlay* WORDSOULOverlay;

	int32 AttackComboCnt;

	bool bIsAttackButtonClickedWhileAttack;

	//임시 테스트용 소켓 변수. 일단 캐릭터에서
	ClientSocket Socket;
	bool bIsConnected;
	uint8 id;
	TUniquePtr<SCCharacterInfo> OtherCharacter;
	
};
