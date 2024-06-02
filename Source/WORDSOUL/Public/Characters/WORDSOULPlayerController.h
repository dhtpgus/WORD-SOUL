// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NetWork/ClientSocket.h"
#include "Characters/WORDSOULCharacter.h"
#include "Enemy/Enemy.h"
#include "NetWork/Packet.h"
#include "Items/Item.h"
#include "Items/Weapons/Weapon.h"
#include "NetWork/WORDSOULGameInstance.h"
#include "WORDSOULPlayerController.generated.h"

/**
 * 
 */

UCLASS()
class WORDSOUL_API AWORDSOULPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AWORDSOULPlayerController();
	virtual ~AWORDSOULPlayerController();

	virtual void Tick(float DeltaTime) override;

	void SpawnMonster(const SCNewEntity& EntityInfo);

	void SendPlayerInfo();
	
	void RecvEntitynfo(const SCNewEntity& EntityInfo);
	void RecvCharacterInfo(const SCPosition& CharacterInfo);
	void RecvMonsterInfo(const SCPosition& MonsterInfo);
	void RecvHpInfo(const SCModifyHP& HpInfo);

	UPROPERTY(BlueprintReadWrite)
	AWORDSOULCharacter* OtherCharacter;

	UPROPERTY(BlueprintReadWrite)
	AEnemy* Monster;

	UPROPERTY(EditDefaultsOnly, Category = Spawn)
	TSubclassOf<class AEnemy> MonsterToSpawn;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	ClientSocket* Socket;
	bool bIsConnected;
	SCNewEntity NewEntityInfo;
	SCPosition OtherCharacterInfo;
	TMap<uint16, SCPosition> EnemyInfoMap;
	SCModifyHP HPInfo;
	
	AItem* OverlappingItem;
	
	void UpdatePlayerInfo(const SCPosition& CharacterInfo);
	void UpdateMonsterInfo(const TMap<uint16, SCPosition>& EnemyInfoMap);
	void UpdateEntityHp(SCModifyHP& hpInfo);

};
